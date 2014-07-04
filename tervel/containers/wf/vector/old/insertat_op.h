


class InsertAt : public OpRecord{
public:

    void *rvalue;
    int pos;
    std::atomic<ShiftHelper<InsertAt> *> next;
    std::atomic<bool> isComplete;

    void init(int p, void *v){
        assert(sizeof(RcObject) <= ALIGNLEN);

        rvalue=v;
        pos=p;
        next=NULL;
        isComplete=false;
        type = dt_insertAt;
    };

    InsertAt(int p, void *v){
        init(p, v);
    };

    void execute(WFVector *vec){
        s_execute(vec);
    }
    bool s_execute(WFVector *vec){
        fcount=0;
        controlWord=(std::atomic<void *> *)&(this->isComplete);

        complete(vec, pos);
        if (next.load() != (ShiftHelper<InsertAt> *)(0x1) ){

            return true;
        }
        else {
            return false; //throw out of bounds exception.
        }
    };

    void cleanup(WFVector *vec, int pos);
    bool complete(WFVector *vec, int pos){
        bool res= completeShift<InsertAt>(this, vec, pos);
        vec->resetMyOp();
        return res;
    };
    bool tryFree();

    void *readThrough(ShiftHelper<InsertAt> *helper);

};

void *InsertAt::readThrough(ShiftHelper<InsertAt> *helper){
  ShiftHelper<InsertAt> *parent=helper->parent;
  if(parent == NULL){
    ShiftHelper<InsertAt>  *temp=next.load();
    assert(temp != NULL);//because op is done
    if(temp == helper){
      return rvalue;
    }
    else{
      return helper->rvalue;
    }
  }
  else{
    ShiftHelper<InsertAt>  *temp=parent->next.load();
    assert(temp != NULL);//because op is done
    if(temp == helper){
      return parent->rvalue;
    }
    else{
      return helper->rvalue;
    }
  }
};


bool InsertAt::tryFree(){
    return shiftTryFree<InsertAt>((InsertAt *)this);
}



void InsertAt::cleanup(WFVector *vec, int pos){

   /* ArrayElement *spot=vec->getSpot(pos);
    void *current=spot->load();
    if (Helper::unmark(current) == next.load()) {
        spot->compare_exchange_strong(current, rvalue);
    }*/ //Already completed in the call to complete for the first guy

    ShiftHelper<InsertAt> *parent=next;
    ShiftHelper<InsertAt> *helper=parent->next.load();
    int i;
    for (i=pos+1; helper != NULL; i++) {

        ArrayElement *spot=vec->getSpot(i);
        void *current=spot->load();
        if (Helper::unmark(current) == helper) {
            spot->compare_exchange_strong(current, parent->rvalue);
        }
        parent=helper;
        helper=parent->next.load();
    }

};

