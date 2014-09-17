#include "tervel/util/info.h"
#include "tervel/util/descriptor.h"
#include "tervel/util/memory/hp/hazard_pointer.h"
#include "tervel/util/memory/rc/descriptor_util.h"

template<class ShiftOp, class T>
class ShiftHelper : public tervel::util::Descriptor {
typedef ShiftHelper<ShiftOp, T> helper_t;
public:
  const helper_t *prev_;
  const ShiftOp *op_rec_;
  const T replaced_value_;
  std::atomic<helper_t *> next_{nullptr};

  ShiftHelper(ShiftOp * op_rec, helper_t *prev, T replaced_value )
    : prev_(prev)
    , op_rec(op_rec_)
    , replaced_value_(replaced_value) {};

  bool complete_phase1(WFVector *vec, int pos);
  bool complete(WFVector *vec, int pos);


  using util::Descriptor::on_watch;
  bool on_watch(std::atomic<void *> *address, void * value) {
    typedef util::memory::hp::HazardPointer::SlotID t_SlotID;
    bool success = util::memory::hp::HazardPointer::watch(
          t_SlotID::SHORTUSE, op_rec_, address, value);

    if (success && prev_ != nullptr) {
      helper_t temp = prev->next_.load();
      if (temp == nullptr) {
        if (prev->next_.compare_exchange_strong(temp, this)) {
          temp = this;
        }
      }

      if (temp != this) {
        success = false;
        address->compare_exchange_strong(value, replaced_value_);
      }

      util::memory::hp::HazardPointer::unwatch(t_SlotID::SHORTUSE);
    }  // End Successfull watch

    if (success) {
      assert(cas_row_->helper_.load() != nullptr);
      assert(util::memory::rc::is_watched(this));
      assert(util::memory::hp::HazardPointer::is_watched(op_rec_));
    }

    return success;
  };



  using util::Descriptor::get_logical_value;
  void * get_logical_value() {
    if (op_rec_->is_complete()) {
      return next_.load()->replaced_value_;
    } else {
      return replaced_value_;
    }
  }

};  // ShiftHelper


template<class ShiftType>
bool completeShift(ShiftType *op, WFVector *vec, int pos);


using util::Descriptor::complete;
void * helper_t::complete(void *value, std::atomic<void *> *address) {



};


template<class ParentOp>
bool ShiftHelper<ParentOp>::complete_phase1(WFVector *vec, int pos){

  if(this->rvalue==NULL){
    main->isComplete.store(true);
    return true;
  }
  ShiftHelper<ParentOp> *lastHelper=this;
  int i;

  for(i=pos+1; ; i++){
    // if(i > vec->csize.load() *2 ){
    //   assert(false);...debugging, but could be hit in a non buggy case
    // }
    if (main->isComplete.load()){
      return true;
    }
    else {
      ArrayElement *spot=vec->getSpot(i);


      while (true) {
        void *current=spot->load();
        ShiftHelper<ParentOp> *tempHelper=lastHelper->next.load();
        if (tempHelper){

          lastHelper=tempHelper;

          if (lastHelper->rvalue == NOT_VALUE) {
            main->isComplete.store(true);
            return true;
          }
          else {
            break;
          }

        }
        else if (Helper::isHelper(current)) {


          Helper * tHelper= Helper::unmark(current);

          if(!tHelper->watch(current, spot)){
            continue;
          }

          if (tHelper->type == type) {
             ShiftHelper<ParentOp> *tHelper2=( ShiftHelper<ParentOp> *)tHelper;
             if(tHelper2->main==main){
              ShiftHelper<ParentOp> *temp=NULL;
              if (lastHelper->next.compare_exchange_strong(temp, tHelper2) || tHelper2 == temp){
                lastHelper=tHelper2;

                if(tHelper2->rvalue == NOT_VALUE){
                  main->isComplete.store(true);
                  tHelper->unwatch();
                  return true;
                }
              }
              else {
                assert(main->isComplete.load()); //Next iteration the first if conditional must be true.
              }

              tHelper->unwatch();
              break;
             }
          }
          else if(tHelper->type == dt_popBack) {
            //This prevents a circular dependency between shift operations and tail operations.
            PopHelper * tHelper2= (PopHelper *)(tHelper);
            void *temp=NULL;
            tHelper2->child.compare_exchange_strong(temp, (void *)0x1);
            spot->compare_exchange_strong(current, NOT_VALUE);
            tHelper->unwatch();
            continue;

          }
          else if(tHelper->type == dt_pushBack) {
            //This prevents a circular dependency between shift operations and tail operations.
            PushHelper * tHelper2= (PushHelper *)(tHelper);

            long tpos=tHelper2->pos.load();
            if(tpos == -1){
              if(tHelper2->pos.compare_exchange_strong(tpos, i)){
                tpos=i;
              }
            }
            tHelper2->complete(vec, i);

            tHelper->unwatch();
            continue;

          }

          else if(tHelper->type >= dt_unknown) {
            assert(false);
          }


          bool helpRes=Helper::remove(vec, i, current);
          if(!helpRes && rDepth>0){
            tHelper->unwatch();
            return false;
          }

          tHelper->unwatch();
          continue;

        }
        else {
          ShiftHelper<ParentOp>* helper=new ShiftHelper<ParentOp>(main, lastHelper, current);
          if (spot->compare_exchange_strong(current, Helper::mark(helper))){

            ShiftHelper<ParentOp> *temp=NULL;
            if (lastHelper->next.compare_exchange_strong(temp, helper) || helper == temp){
              lastHelper=helper;

              if(current == NOT_VALUE){
                main->isComplete.store(true);
                return true;
              }
              break;
            }
            else {
              void *temp2=Helper::mark(helper);
              spot->compare_exchange_strong(temp2, current);
              helper->safeFree(); //was globally visable
              assert(main->isComplete.load()); //Next iteration the first if conditional must be true.
              return true;
            }

          }
          else {
            //The valu at the address changed, and so did the value of current. So re-evaluate current...
            helper->unsafeFree();//Was never globally visable
          }

        }//End Else it is a valid position

      }//End While true loop

    }//End Else it is not compelte

  }//End For Loop.

  assert(false);
  return false;
}//End Complete Phase 1


template<>
bool ShiftHelper<InsertAt>::complete(WFVector *vec, int pos){
  ArrayElement *spot=vec->getSpot(pos);
  void *current=Helper::mark(this);

  ShiftHelper<InsertAt> *temp=NULL;
  if(parent == NULL){
    if(main->next.compare_exchange_strong(temp, this) || temp==this){
      if(!complete_phase1(vec, pos)){
        assert(rDepth > 0 || main->isComplete.load());
        return false;
      }
      assert(main->isComplete.load());
      spot->compare_exchange_strong(current, main->rvalue);

    }
    else{
      assert(main->isComplete.load() || temp ==(ShiftHelper<InsertAt> *)0x1);
      spot->compare_exchange_strong(current, rvalue);

    }

  }
  else{
    if(parent->next.compare_exchange_strong(temp, this) || temp==this){
      if(!complete_phase1(vec, pos)){
        assert(rDepth > 0 || main->isComplete.load());
        return false;
      }
      assert(main->isComplete.load());
      spot->compare_exchange_strong(current, parent->rvalue);

    }
    else{
      assert(main->isComplete.load() || temp ==(ShiftHelper<InsertAt> *)0x1);
      spot->compare_exchange_strong(current, rvalue);

    }
  }
  return true;
}

template<>
bool ShiftHelper<EraseAt>::complete(WFVector *vec, int pos){
  ArrayElement *spot=vec->getSpot(pos);
  void *current=Helper::mark(this);

  ShiftHelper<EraseAt> *temp=NULL;

  bool success;

  if (parent == NULL) {
    success=main->next.compare_exchange_strong(temp, this) || temp==this;
  }
  else {
    success=parent->next.compare_exchange_strong(temp, this) || temp==this;
  }

  if (success) {
    if(!complete_phase1(vec, pos)){
      assert(rDepth > 0 || main->isComplete.load());
      return false;
    }

    assert(main->isComplete.load());

    if(next.load() == NULL){
      spot->compare_exchange_strong(current, NOT_VALUE);
    }
    else{
      spot->compare_exchange_strong(current, next.load()->rvalue);
    }

  }
  else {
    assert(main->isComplete.load() || temp ==(ShiftHelper<EraseAt> *)0x1);
    spot->compare_exchange_strong(current, rvalue);
  }

  return true;
}



template<class ShiftType>
bool completeShift(ShiftType *op, WFVector *vec, int pos){
  if(pos >= vec->csize){
    ShiftHelper<ShiftType> *temp=NULL;
    if(op->next.compare_exchange_strong(temp, (ShiftHelper<ShiftType> *)(0x1))){
      op->isComplete.store(true);
      return true;
    }

  }


  while (true) {
    if(fcount++==MAX_FAILURES){
      if(rDepth == 0){
        vec->announceOp(op);
      }
      else{
        assert(false);
        return false;
      }
    }


    ShiftHelper<ShiftType> *temp=op->next.load();
    if (temp == (ShiftHelper<ShiftType> *)0x1) {
      op->isComplete.store(true);
      return true;
    }
    else if (temp != NULL) {
      temp->complete(vec,pos);
      return true;
    }

    ArrayElement *spot=vec->getSpot(pos);
    void *current=spot->load();
    if (Helper::isHelper(current)) {

      Helper * tHelper= Helper::unmark(current);
      if(!tHelper->watch(current, spot)){
        continue;
      }

      if (tHelper->type == dt_shifthelper) {
         ShiftHelper<ShiftType> *tHelper2=( ShiftHelper<ShiftType> *)tHelper;
         if(tHelper2->main==op){
          ShiftHelper<ShiftType> * temp=NULL;
          if (op->next.compare_exchange_strong(temp, tHelper2) || tHelper2 == temp){

            if(tHelper2->rvalue == NOT_VALUE){
              //main->isComplete.store(true);
              assert(false);
              op->isComplete.store(true);
              tHelper->unwatch();
              return true;;
            }
            else{
              tHelper2->complete(vec,pos);
            }
          }

          assert(op->next.load());
          if (temp == (ShiftHelper<ShiftType> *)0x1) {
            op->isComplete.store(true);

          }
          tHelper->unwatch();
          return true;
         }//End helper for this op
      }//End If it is a insertAt descripor
      /* No need since we have not placed anything yet
      else if(tHelper->type == dt_popBack) {
        //This prevents a circular dependency between shift operations and tail operations.
        PopHelper * tHelper2= (PopHelper *)(tHelper);
        void *temp=NULL;
        tHelper2->child.compare_exchange_strong(temp, (void *)0x1);
        spot->compare_exchange_strong(current, NOT_VALUE);
        continue;

      }*/
      else if(tHelper->type >= dt_unknown) {
        assert(false);
        op->isComplete.store(true);
        tHelper->unwatch();
        return false;;
      }
      Helper::remove(vec, pos, current);
      tHelper->unwatch();

      continue;

    }
    else if (current == NOT_VALUE) {
      //temp=NULL; already null
      if (op->next.compare_exchange_strong(temp, (ShiftHelper<ShiftType> *)(0x1)) || temp == (ShiftHelper<ShiftType> *)0x1) {
        op->isComplete.store(true);
        return true;
      }
      else {
        temp->complete(vec, pos);
        return true;
      }
    }
    else {
      ShiftHelper<ShiftType>* helper=new ShiftHelper<ShiftType>(op, NULL, current);
      if (spot->compare_exchange_strong(current, Helper::mark(helper))) {
        helper->complete(vec,pos);
        if (op->next.load() != helper) {
          helper->safeFree(); //was not associated because op was already done when it was placed.
        }
        return true;
      }
      else {
        //The value at the address changed, So re-evaluate current (which changes on a failed compare and exchange)...
        helper->unsafeFree();//Was never globally visable.
      }
    }//End Else it is a valid value
  }//End While true loop

  assert(false);
};//End begin complete
