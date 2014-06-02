#ifndef __VECTOR_PUSH_OP_H_
#define __VECTOR_PUSH_OP_H_

template<class T>
class PushOp: public OpRecord {
 public:
  std::atomic<PushHelper<T> *> helper_{nullptr};
  std::atomic<size_t> pos_{-1};
  const Vector vector_
  const T value_;
  PushOp(Vector vector, T value)
  : vector_(vector)
  , value_(value) {};

  size_t begin();

  void execute();

};

template<class T>
size_t PushOp::begin(){
  tl_control_word = (std::atomic<void *> *)(&(helper_));

  size_t pushed_pos = vector_->size();

  while(true){

    if (pushedPos <0 ) {
      pushedPos = 0;
    }

    if (fcount++ < util::ProgressAssurance::MAX_FAILURES) {
      util::ProgressAssurance::make_announcement(
          reinterpret_cast<tervel::util::OpRecord *>(op));

      pushed_pos = op->result();
      op->safe_delete();
      return pushed_pos;
    }

    ArrayElement *spot=vec->getSpot(pushedPos);
    void *expected= spot->load();

    if( expected==NOT_VALUE){

      if(pushedPos==0){
        if(spot->compare_exchange_strong(expected, value)){
          return 0;
        }
        else{
          pushedPos++;
          spot=vec->getSpot(pushedPos);
        }
      }

      assert(pushedPos >=1);

      PushHelper *ph=new PushHelper(value);

      if(spot->compare_exchange_strong(expected, Helper::mark(ph))  ){

        ph->complete(vec, pushedPos);
        long res=ph->pos.load();
        ph->safeFree();
        assert(res != -1);
        if(res != -2 ){//returns true if was success
          return pushedPos;
        }
        else{//must have shrunk
          pushedPos--;
          continue;
        }
      }
    }
    else if(Helper::isHelper(expected)){
      Helper *tHelper=Helper::unmark(expected);
      if(!tHelper->watch(expected, spot)){
        continue;
      }

      bool helpRes=Helper::remove(vec, pushedPos,expected );
      tHelper->unwatch();
      assert(rDepth == 0);

      if(!helpRes){
        vec->announceOp(this);
        long res= this->s_execute(vec);
        vec->resetMyOp();
        return res;
      }
      continue;
    }
    else{//its value
      pushedPos++;
    }
  }//End While
}


template<class T>
size_t PushOp::s_execute(WFVector *vec){//only this op can call it

  controlWord=(std::atomic<void *> *)(&(assoc));

  long pushedPos=vec->csize.load();
  while(this->assoc.load() == NULL){// Implicit BOUND by threads
    assert(rDepth == 0);
    if (pushedPos <0 )
      pushedPos=0;

    ArrayElement *spot=vec->getSpot(pushedPos);
    void *expected= spot->load();

    if( expected==NOT_VALUE){

      PushHelper *ph=new PushHelper(value, this);
      if(spot->compare_exchange_strong(expected, Helper::mark(ph))  ){

        ph->complete(vec, pushedPos);
        long res=ph->pos;
        assert(res != -1);
        if(pos != -2){
          return res;
        }
        else{
          pushedPos--;
          ph->safeFree();
          continue;
        }

      }//End If successful Cas
    }
    else if(Helper::isHelper(expected)){
      Helper *tHelper=Helper::unmark(expected);
      if(!tHelper->watch(expected, spot)){
        continue;
      }
      Helper::remove(vec, pushedPos, expected );
      tHelper->unwatch();
      continue;
    }
    else{//its value
      pushedPos++;
    }
  }//End While

  return this->assoc.load()->pos;
}


bool PushOp::tryFree(){
  if(this->isWatched()){
    return false;
  }
  else{
    PushHelper *ph= assoc.load();
    if (ph && ph->isWatched() )
      return false;
    else if(ph)
      ph->unsafeFree();

    this->unsafeFree();
    return true;
  }
}


#endif  // __VECTOR_PUSH_OP_H_