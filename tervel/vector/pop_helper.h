

class PopHelper: public Helper{
public:
    std::atomic<void *> child;
    PopOp *controlOp;
    void init(PopOp *cop){
        assert(sizeof(PopHelper) <= ALIGNLEN);
        child=NULL;
        type=dt_popBack;
        controlOp=cop;
    }
    PopHelper(){
        init(NULL);
    };
    PopHelper(PopOp *cop){
        init(cop);
    };

    bool complete(WFVector *vec, int pos);

    bool getResult(void *&v);
    bool tryFree();


    bool watch(void *p, ArrayElement *a){

        rc_count.fetch_add(1);

        if (a->load() != p){
            rc_count.fetch_add(-1);
            return false;
        }


        if (controlOp){
            controlOp->rc_count.fetch_add(1);
            if (a->load() != p){
                rc_count.fetch_add(-1);
                controlOp->rc_count.fetch_add(-1);
                return false;
            }
        }

        return true;
    };

    void unwatch(){
        rc_count.fetch_add(-1);

        if (controlOp){
            controlOp->rc_count.fetch_add(-1);
        }
    };
    bool isWatched(){
        return (rc_count.load() !=0);
    };

    void * readThrough(){
        return NOT_VALUE;
    };

};


class PopSubHelper: public Helper{
public:
    void *value;
    PopHelper * parent;

    void init(PopHelper *p){
        assert(sizeof(PopSubHelper) <= ALIGNLEN);

      parent=p;
        type=dt_popBackH;
        value=NOT_VALUE;
    };

    PopSubHelper(PopHelper *p){
        init(p);
    };

    void * getValue();
    bool complete(WFVector *vec, int pos);
    bool tryFree();

    bool watch(void *p, ArrayElement *a){

        rc_count.fetch_add(1);

        if (a->load() != p){
            rc_count.fetch_add(-1);
            return false;
        }

        parent->rc_count.fetch_add(1);
        if (a->load() != p){
            rc_count.fetch_add(-1);
            parent->rc_count.fetch_add(-1);
            return false;
        }

        if (parent->controlOp){
            parent->controlOp->rc_count.fetch_add(1);
            if (a->load() != p){
                parent->controlOp->rc_count.fetch_add(-1);
                rc_count.fetch_add(-1);
                parent->rc_count.fetch_add(-1);
                return false;
            }
        }

        return true;
    };

    void unwatch(){
        rc_count.fetch_add(-1);
        parent->rc_count.fetch_add(-1);
        if (parent->controlOp){
            parent->controlOp->rc_count.fetch_add(-1);
        }
    };
    bool isWatched(){
        return (rc_count.load() !=0);
    };

    void * readThrough(){
      return getValue();
    };

};

void * PopSubHelper::getValue(){

    void *expected=parent->child.load();
    if (expected== NULL){
        if (parent->child.compare_exchange_strong(expected,  (void *)this)){
            expected=this;
        }
    }


    if (expected == this){
        if(parent->controlOp != NULL ){
            if(parent->controlOp->assoc.load()==parent){
                return NOT_VALUE;
            }
        }
        else{
            return NOT_VALUE;
        }


    }
    return value;

}



bool PopSubHelper::complete(WFVector *vec, int pos){
    assert(type == dt_popBackH );
    void *newValue;
    void *e=NULL;

    PopHelper *t=NULL;
    if( (parent->child.compare_exchange_strong(e, (void *)this) || e==this) ){
        if(parent->controlOp == NULL){
            newValue=NOT_VALUE;
        }
        else if (parent->controlOp->assoc.compare_exchange_strong(t, parent) || (t==parent) ){
            newValue=NOT_VALUE;
        }
        else{
            newValue=value;
        }

    }
    else {
        newValue=value;
    }

    ArrayElement *spot=vec->getSpot(pos);

    void *temp=Helper::mark(this);
    spot->compare_exchange_strong(temp, newValue);
    return true;

};

bool PopHelper::getResult(void *&v){
    if (child.load()!= (void *)0x1 ) {

        if(controlOp){
            PopHelper *t=NULL;
            if(controlOp->assoc.compare_exchange_strong(t, this) || t==this){
                PopSubHelper *temp=(PopSubHelper *)(child.load());
                v=temp->value;
                return true;
            }
        }
        else{
            PopSubHelper *temp=(PopSubHelper *)(child.load());
            v=temp->value;
            return true;
        }

    }
    return false;
}



bool PopHelper::complete(WFVector *vec, int pos){

    assert(type == dt_popBack );

    PopSubHelper *psh=new PopSubHelper(this);

    ArrayElement *pb=vec->getSpot(pos-1);

    while (child.load() == NULL) {

        void *expected=pb->load();

        if (Helper::isHelper(expected)) {// TO BOUND

            Helper *tHelper=Helper::unmark(expected);
            if(!tHelper->watch(expected, pb)){
                continue;
            }

            bool helpRes=Helper::remove(vec, pos-1, expected);

            tHelper->unwatch();

            if(!helpRes && rDepth>0){
                psh->unsafeFree();
                return false;
            }
            else {
                continue;
            }
        }


        if (expected==NOT_VALUE){
            void *c_child=NULL;
            if (child.compare_exchange_strong(c_child,(void *)0x1)){
                break;
            }
        }
        else {
            psh->value=expected;
            if (pb->compare_exchange_strong(expected, Helper::mark(psh))){
                psh->complete(vec,pos-1);
                break;
            }
            else {
                continue;
            }

        }
    }//End While

    ArrayElement *pa=vec->getSpot(pos);

    void *temp=Helper::mark(this);
    pa->compare_exchange_strong(temp, NOT_VALUE);

    if(child.load() != psh)
        psh->safeFree();
    return true;

}


bool PopOp::tryFree(){
    if(this->isWatched()){
        return false;
    }
    else{
        PopHelper *ph= assoc.load();
        if (ph!=NULL && ph!=(PopHelper *)(0x1) ){
            if(ph->isWatched() )
                return false;

            PopSubHelper *pch=(PopSubHelper *)ph->child.load();
            if(pch!=NULL && pch != (PopSubHelper *)0x1){
                if (pch->isWatched()){
                    return false;
                }
                pch->unsafeFree();
            }
            ph->unsafeFree();
        }

        this->unsafeFree();
        return true;
    }
}

bool PopHelper::tryFree(){

    if(this->isWatched()){
        return false;
    }
    else{
        PopHelper *op= (PopHelper *)(this);
        PopSubHelper *t=(PopSubHelper *)(op->child.load());
        if(t != NULL && t != (PopSubHelper *)0x1 ){
            if(t->isWatched()){
                return false;
            }
            else{
                t->unsafeFree();
            }
        }
        this->unsafeFree();
        return true;

    }
}
bool PopSubHelper::tryFree(){
    if(this->isWatched()){
        return false;
    }
    else{
        this->unsafeFree();
        return true;
    }
}
