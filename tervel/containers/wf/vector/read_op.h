class ReadOp: public OpRecord{
public:
    std::atomic<void *>value;
    int pos;
    ReadOp(int p){
        assert(sizeof(RcObject) <= ALIGNLEN);
        type=dt_readop;
        pos=p;
        value.store(NULL);
    };

    bool s_execute(WFVector *vec, void *&v){

        ArrayElement *spot=vec->getSpot(pos);

        void * aValue=value.load();
        while (aValue == NULL){
            void *cvalue=spot->load(std::memory_order_relaxed);

            if (Helper::isHelper(cvalue)){
                Helper *tHelper=Helper::unmark(cvalue);
                if (tHelper->watch(cvalue, spot)){
                    Helper::remove(vec, pos, cvalue);
                    tHelper->unwatch();
                }
                aValue=value.load();
            }
            else if (cvalue== NOT_VALUE){
                value.compare_exchange_strong(aValue, (void *)(0x1));
                break;
            }
            else{
                assert(Value::isValid(cvalue));
                value.compare_exchange_strong(aValue, cvalue);
                break;
            }
        }

        aValue=value.load();
        if (aValue == (void *)0x1){
            return false;
        }
        else{
            assert(aValue != NOT_VALUE);
            v=aValue;
            return true;
        }

    };


    void execute(WFVector *vec){
        void *g;
        s_execute(vec, g);
    };

    bool tryFree();

    bool watch(void *p, ArrayElement *a){

        rc_count.fetch_add(1);

        if (a->load() != p){
            rc_count.fetch_add(-1);
            return false;
        }
        return true;
    };

    void unwatch(){
        rc_count.fetch_add(-1);
    };
    bool isWatched(){
        return (rc_count.load() !=0);
    };

};


bool ReadOp::tryFree(){
    if(this->isWatched()){
        return false;
    }
    else{
        this->unsafeFree();
        return true;
    }
}

