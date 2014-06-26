
class PushHelper: public Helper{
public:
    void *value;
    std::atomic<long> pos;
    PushOp *controlOp;
    void init(void *v, PushOp *op){
        assert(sizeof(PushHelper) <= ALIGNLEN);
        value=v;
        pos=-1;
        type=dt_pushBack;
        controlOp=op;
    };

    PushHelper(void *v){
        init(v, NULL);
    };

    PushHelper(void *v, PushOp *op){
        init(v,op);
    };



    bool complete(WFVector *vec, int pos);
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
      if(pos.load() >= 0){
        return value;
      }
      else{
        return NOT_VALUE;
      }
    };

};


bool PushHelper::complete(WFVector *vec, int pos){
    assert(type == dt_pushBack);

    ArrayElement *pA=vec->getSpot(pos);
    ArrayElement *pB=vec->getSpot(pos-1);

    long spos=this->pos.load();
    while (this->pos.load() == -1) {

        if (fcount++ == MAX_FAILURES){
            if(rDepth > 0){
                return false;
            }

            this->pos.compare_exchange_strong(spos, -2);
            break;
        }

        void *expected=pB->load();

        if (Helper::isHelper(expected)) {
            Helper *tHelper=Helper::unmark(expected);

            if (!tHelper->watch(expected, pB)) {
                continue;
            }
            bool helpRes = Helper::remove(vec, pos-1, expected);
            tHelper->unwatch();

            if (!helpRes && rDepth>0) {
                return false;
            }
            else{
                continue;
            }
        }

        assert(!Helper::isHelper(expected));
        if (expected == NOT_VALUE) {
            this->pos.compare_exchange_strong(spos, -2);
        }
        else{
            this->pos.compare_exchange_strong(spos, pos);
        }
        break;

    }
    spos=this->pos.load();
    assert(spos != -1);

    PushOp *op=this->controlOp;
    PushHelper *temp=NULL;
    void *expected=Helper::mark(this);

    if(spos== -2){
        pA->compare_exchange_strong(expected, NOT_VALUE);
    }
    else if(op == NULL){
        pA->compare_exchange_strong(expected, value);
    }
    else if (op->assoc.compare_exchange_strong(temp, this) || temp==this){
        pA->compare_exchange_strong(expected, value);
    }
    else{
        pA->compare_exchange_strong(expected, NOT_VALUE);
    }

    return true;

};

bool PushHelper::tryFree(){
    if(this->isWatched()){
        return false;
    }
    else{
        this->unsafeFree();
        return true;
    }
}
