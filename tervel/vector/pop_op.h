
class PopOp:public OpRecord{
public:
    std::atomic<PopHelper *> assoc;

    PopOp(){
        assert(sizeof(PopOp) <= ALIGNLEN);
        type=dt_popOp;
        assoc.store(NULL);
    }

    void * begin(WFVector *vec);

    void * s_execute(WFVector *vec);

    void execute(WFVector *vec){
        s_execute(vec);
        return;
    };

    bool tryFree();

};





void * PopOp::begin(WFVector *vec){

    PopHelper *ph=new PopHelper();
    controlWord=&(ph->child);

    int popPos=vec->csize.load();
    while(true){// TO BOUND

        if(fcount++ == MAX_FAILURES){
            ph->unsafeFree();
            vec->announceOp(this);
            void *res= this->s_execute(vec);
            vec->resetMyOp();
            return res;
        }

        if(popPos<=0){
            ph->unsafeFree();
            return NOT_VALUE;
        }

        ArrayElement *spot=vec->getSpot(popPos);
        void *expected= spot->load();

        if( expected==NOT_VALUE){
            if(spot->compare_exchange_strong(expected, Helper::mark(ph))    ){

                ph->complete(vec, popPos);

                void *value;
                if(ph->getResult(value)){//returns true if was success

                    assert(Value::isValid(value));

                    ph->safeFree();

                    return value;
                }
                else{//must have shrunk
                    ph->safeFree();
                    ph=new PopHelper();
                    controlWord=&(ph->child);
                    popPos--;
                    continue;
                }
            }

        }
        else if(Helper::isHelper(expected)){
            Helper *tHelper=Helper::unmark(expected);

            if(!tHelper->watch(expected, spot)){
                continue;
            }
            bool helpRes=Helper::remove(vec, popPos,expected );
            tHelper->unwatch();

            assert(rDepth==0);

            if(!helpRes){
                ph->unsafeFree();
                vec->announceOp((void *)this);
                void *res= this->s_execute(vec);
                vec->resetMyOp();
                return res;
            }
            continue;
        }
        else{//its value
            popPos++;
        }
    }//End While

}

void * PopOp::s_execute(WFVector *vec){

    PopHelper *ph=new PopHelper(this);
    controlWord=(std::atomic<void *> *)&(assoc);

    int popPos=vec->csize.load();
    while(true){

        if(popPos<=0){
            ph->unsafeFree();

            PopHelper *temp1=NULL;
            PopHelper *temp2=(PopHelper *) 0x1;
            if(assoc.compare_exchange_strong(temp1, temp2) || temp1 == temp2){
                return NOT_VALUE;
            }
            else{
                void *value;
                bool res=temp1->getResult(value);
                assert(res);
                return value;
            }

        }

        ArrayElement *spot=vec->getSpot(popPos);
        void *expected= spot->load();

        if( expected==NOT_VALUE){
            if (spot->compare_exchange_strong(expected, Helper::mark(ph)) ) {

                ph->complete(vec, popPos);
                void *value;
                if(ph->getResult(value)){//returns true if was success
                    assert(Value::isValid(value));
                    return value;
                }
                else{//must have shrunk
                    ph->safeFree();
                    ph=new PopHelper(this);
                    popPos--;
                    continue;
                }
            }

        }
        else if (Helper::isHelper(expected)) {
            Helper *tHelper=Helper::unmark(expected);
            if(!tHelper->watch(expected, spot)){
                continue;
            }

            Helper::remove(vec, popPos,expected);
            tHelper->unwatch();

            assert(rDepth==0);
            continue;
        }
        else {//its value
            popPos++;
        }
    }//End While

}
