#ifndef __wfv_class_fun__
#define __wfv_class_fun__

#include "wfv_macros.hpp"
#include "wfv_classes.hpp"
#include "WFVector.hpp"

long PushOp::begin(WFVector *vec){

    
    controlWord=(std::atomic<void *> *)(&(assoc));

    long pushedPos=vec->csize.load();

    int lcount=0;
    while(true){ lcount++;
        if(pushedPos > vec->csize.load()+100){
            lcount=lcount;
            assert(false);
        }
        if (pushedPos <0 )
            pushedPos=0;

        assert(rDepth == 0);
        if(fcount++ == MAX_FAILURES){
            assert(rDepth==0);
            vec->announceOp(this);
            long res= this->s_execute(vec);            
            vec->resetMyOp();
            return res;

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
            
            if(spot->compare_exchange_strong(expected, Helper::mark(ph))    ){

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


long PushOp::s_execute(WFVector *vec){//only this op can call it
    
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
            if(spot->compare_exchange_strong(expected, Helper::mark(ph))    ){

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
        //     assert(false);...debugging, but could be hit in a non buggy case
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

void EraseAt::cleanup(WFVector *vec, int pos){

   /* ArrayElement *spot=vec->getSpot(pos);
    void *current=spot->load();
    if (Helper::unmark(current) == next.load()) {
        spot->compare_exchange_strong(current, rvalue);
    }*/ //Already completed in the call to complete for the first guy

    ShiftHelper<EraseAt> *parent=next;
    ShiftHelper<EraseAt> *helper=parent->next.load();
    int i;
    for (i=pos+1; helper != NULL; i++) {

        ArrayElement *spot=vec->getSpot(i);
        void *current=spot->load();
        if (Helper::unmark(current) == helper) {
            if ( helper->next.load()== NULL) {
               spot->compare_exchange_strong(current, NOT_VALUE);
            }
            else {
               spot->compare_exchange_strong(current, helper->next.load()->rvalue);
            }
        }
        parent=helper;
        helper=helper->next.load();
    }

};

template<class ShiftType>
bool shiftTryFree(ShiftType *op){
	
	if(op->isWatched()){
		return false;
	}
	else{
		 ShiftHelper<ShiftType> *helper=op->next.load();
		 while(helper != NULL && helper != ( ShiftHelper<ShiftType> *)0x1){
		 	if(helper->isWatched()){
		 		return false;
		 	}
		 	else{
		 		helper=helper->next.load();
		 	}
		 }

		 helper=op->next.load();
		 while(helper != NULL && helper != ( ShiftHelper<ShiftType> *)0x1){
		 	helper->unsafeFree();//Does not zero or change internal data
			helper=helper->next.load();//So this line is safe
		 }
		 
		 op->unsafeFree();

		 return true;

	}
}



bool InsertAt::tryFree(){
	return shiftTryFree<InsertAt>((InsertAt *)this);
}

bool EraseAt::tryFree(){
	return shiftTryFree<EraseAt>((EraseAt *)this);
}
bool PushHelper::tryFree(){
	if(this->isWatched()){
		return false;
	}
	else{
		this->unsafeFree();
		return true;
	}
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

bool ReadOp::tryFree(){
	if(this->isWatched()){
		return false;
	}
	else{
		this->unsafeFree();
		return true;
	}
}

template<class ParentOp>
bool ShiftHelper<ParentOp>::tryFree(){
	if(this->isWatched()){
		return false;
	}
	else{
		this->unsafeFree();
		return true;
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


#endif