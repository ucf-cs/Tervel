


class CasHelper: public tervel::util::Descriptor {
public:
    CasOp *op;
    CasHelper(CasOp * cop){
        assert(sizeof(RcObject) <= ALIGNLEN);
        type=dt_cashelper;
        op=cop;

    }
    bool complete(WFVector *vec, int pos);
    bool watch(void *p, ArrayElement *a);
    void unwatch();
    bool isWatched();

    bool tryFree(){
        if (isWatched()){
            return false;
        }
        else{
            this->unsafeFree();
            return true;
        }
    };

    void * readThrough();
};



void * CasHelper::readThrough(){
  void *temp=op->assoc.load();
  if(Helper::mark(this) == temp){
    return op->n_value;
  }
  else{
    return op->o_value;
  }
}




bool CasHelper::complete(WFVector *vec, int pos){
    ArrayElement *spot=vec->getSpot(pos);

    void *temp=NULL;
    void *tempMarked= Helper::mark(this);
    if (op->assoc.compare_exchange_strong(temp, tempMarked) || temp == tempMarked){
        spot->compare_exchange_strong(tempMarked, op->n_value);
    }
    else{
        spot->compare_exchange_strong(tempMarked, op->o_value);
    }
    return true;

}

bool CasHelper::watch(void *p, ArrayElement *a){

    rc_count.fetch_add(1);

    if(a->load() != p){
        rc_count.fetch_add(-1);
        return false;
    }
    op->rc_count.fetch_add(1);

    if(a->load() != p){
        rc_count.fetch_add(-1);
        op->rc_count.fetch_add(-1);
        return false;
    }

    return true;
};

void CasHelper::unwatch(){
    rc_count.fetch_add(-1);
    op->rc_count.fetch_add(-1);
};
bool CasHelper::isWatched(){
    return (rc_count.load() !=0);
};