#ifndef __WFVECTOR_CLASSES__
#define  __WFVECTOR_CLASSES__ 1

#include "wfv_macros.hpp"
#include "WFVector.hpp"


class Value{
public:
    static bool isAlive(void *p){
        return !isDead(p);
    }
    static bool isDead(void *p){
        if ((long)p == NOT_ALIVE_VALUE){
            return false;
        }
        if (    (((long)p)&(0x1)) ==0    ){
            return false;
        }
        
        return true;
    }
    
    static bool isValid(void *p){
        if ((long)p==NOT_VALUE){
            return false;
        }
        return (((long)(p))&(0x3))==(0x0);
    }
    
    static bool getDead(void *p){
        return (void *)(((long)(p))|(0x2));
    }
    static bool getUnDead(void *p){
        return (void *)(((long)(p))&(~0x2));
    }
    
    static bool isNotAlive(void *p){
        return ((long)p==NOT_ALIVE_VALUE);
    }
};

enum DescriptorType {dt_insertAt, dt_eraseAt, dt_shifthelper, 
                     dt_popOp,    dt_popBack, dt_popBackH,
                     dt_pushOp,   dt_pushBack,
                     dt_casop,    dt_cashelper, 
                     dt_readop,   dt_unknown};


class RcObject;

__thread RcObject * tl_safe_pool=NULL;
__thread RcObject * tl_unsafe_pool=NULL;



class RcObject{
public:
	DescriptorType type=dt_unknown;

	std::atomic<long> rc_count;
	std::atomic<RcObject *> rc_next;

#ifdef DEBUG
    std::atomic<long> allcount;
    std::atomic<long> freecount;
#endif

	RcObject(){}

    void init(){
        assert(sizeof(RcObject) <= ALIGNLEN);
		rc_count=0;
		rc_next=NULL;
        
        #ifdef DEBUG
            allcount=1;
            freecount=0;
        #endif
	};

    
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


	bool safeFree(){
		if (tryFree()){
			//Internall calls unsafeFree()
			return true;
		}
		else{
        #ifdef DEBUG
            freecount.fetch_add(1);
            assert(freecount.load() == allcount.load());
        #endif
			this->rc_next=tl_unsafe_pool;
			tl_unsafe_pool=this;

			return false;
		}

	};

	void unsafeFree(){
		this->rc_next=tl_safe_pool;
		tl_safe_pool=this;

        #ifdef DEBUG
            freecount.fetch_add(1);
            assert(freecount.load() == allcount.load());
        #endif
	};

#if 0
    void* operator new (size_t size)
    {
		while (tl_unsafe_pool){
			RcObject *temp=tl_unsafe_pool->rc_next;

			if (tl_unsafe_pool->tryFree()){
				tl_unsafe_pool=temp;
			}
			else{
				break;
			}
		}
        if(tl_unsafe_pool){
    		RcObject *prev=tl_unsafe_pool;
    		RcObject *temp=tl_unsafe_pool->rc_next;

    		while (temp){
    			RcObject *temp3=temp->rc_next;
    			if (temp->tryFree()){
                    prev->rc_next=temp3;
    				temp=temp3;
    			}
    			else{
    				prev=temp;
    				temp=temp3;
    			}
    		};
        }

		if (tl_safe_pool) {
			RcObject *temp=tl_safe_pool;
			tl_safe_pool=tl_safe_pool->rc_next;

            #ifdef DEBUG
                assert(temp->freecount.load() == temp->allcount.load());
                temp->allcount.fetch_add(1);
            #endif

			return temp;
		}
		else {
            void *p= (void *)new char[ALIGNLEN];
            if (p==NULL) {
                exit(-1);
            }
            else {
                ((RcObject *)p)->init();
                return p;
            }
		}

	};
#endif

	virtual bool tryFree(){assert(false); return false;};
};

class Helper: public RcObject{
public:
    //TODO in isHelper add resize bit check as well...
    

    static bool isHelper(void *p){
        if ((long)p==DEAD_NOT_VALUE){
            return false;
        }
        else if ((long)p==NOT_ALIVE_VALUE){
            return false;
        }
        else {
            return ( (((long)p) & 0x1)== 0x1 );
        }
    };
    
    static Helper* unmark(void *p){
        return (Helper *)(((long)(p))&(~0x1));
    };
    
    static void * mark(Helper *p){
        assert(long(p) > 0x5000);
        return (void *)(((long)(p))|(0x1));
        
    };

    static bool remove(WFVector *vec, int pos, void *p){// TO BOUND RECURSIVE
        
        assert(controlWord);
        if ( rDepth > vec->vector_nThreads*2 || controlWord->load() ){
            return false;
        }
        rDepth++;

        assert(isHelper(p));

        Helper *helper= unmark(p);
        assert(helper->type < dt_unknown);
      
        bool success=helper->complete(vec, pos);
        

        rDepth--;

        return success;
        
    }

    virtual bool complete(WFVector *vec, int pos){ assert(false); return false; };
    
    virtual bool tryFree(){ assert(false); return false; };
    virtual void * readThrough(){ assert(false); return NULL; };

};

class OpRecord: public RcObject{
public:
    virtual void execute(WFVector *vec){};
    virtual bool tryFree(){ assert(false); return false; };

};

class CasOp;

class CasHelper: public Helper{
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

class CasOp: public OpRecord{
public:
    std::atomic<void *> assoc;
    int pos;
    void *o_value;
    void *n_value;

    CasOp(int p, void *o, void *n){
        assert(sizeof(RcObject) <= ALIGNLEN);
        type=dt_casop;
        o_value=o;
        n_value=n;
        pos=p;
        assoc.store(NULL);
    }

    bool s_execute(WFVector *vec, void *&v){
        ArrayElement *spot=vec->getSpot(pos);
        
        CasHelper *cah=new CasHelper(this); 

        void *ahelper=assoc.load();
        while ( ahelper == NULL){
            void *cvalue=spot->load(std::memory_order_relaxed);

            if (Helper::isHelper(cvalue)){
                Helper *tHelper=Helper::unmark(cvalue);
                if (tHelper->watch(cvalue, spot)){
                    Helper::remove(vec, pos, cvalue);
                    tHelper->unwatch();
                }
                ahelper=assoc.load();
                continue;
            }

            else if (cvalue == o_value){   
                
                void *tempMarked=Helper::mark(cah);
                if (spot->compare_exchange_strong(cvalue,tempMarked )){
                    if (assoc.compare_exchange_strong(ahelper, tempMarked) || ahelper == tempMarked){
                        spot->compare_exchange_strong(tempMarked, n_value);
                    }
                    else{
                        spot->compare_exchange_strong(tempMarked, o_value);
                        cah->safeFree();
                    }
                    break;
                }
                
            }
            else{
                assoc.compare_exchange_strong(ahelper, cvalue);
                cah->unsafeFree();
                break;
            }
        }//End while

        ahelper=assoc.load();
        if (Helper::isHelper(ahelper)){
            return true;
        }
        else{
            v=ahelper;
            return false;
        }

    }

    void execute(WFVector *vec){
        void *t;
        s_execute(vec, t);
    };
    
    bool tryFree(){
        if (isWatched()){
            return false;
        }
        void *t=assoc.load();
        if (Helper::isHelper(t)) {
            Helper *tHelper=Helper::unmark(t);
            if (tHelper->isWatched()) {
                return false;
            }
            else{
                tHelper->unsafeFree();
            }
        }

        this->unsafeFree();
        return true;
    };


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

void * CasHelper::readThrough(){
	void *temp=op->assoc.load();
	if(Helper::mark(this) == temp){
		return op->n_value;
	}
	else{
		return op->o_value;
	}
}

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

class PushHelper;
class PopHelper;

class PushOp:public OpRecord{
public:
    std::atomic<PushHelper *> assoc;

    long pos;
    void *value;
    PushOp(void *v){
        assert(sizeof(PushOp) <= ALIGNLEN);
        type=dt_pushOp;
        value=v;
        assoc.store(NULL);
        pos=-2;
    }

    long begin(WFVector *vec);

    long s_execute(WFVector *vec);

    void execute(WFVector *vec){
        s_execute(vec);
        return;
    };

    bool tryFree();


};

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




template<class ParentOp>
class ShiftHelper : public Helper {
public:
    void *rvalue;
    std::atomic<ShiftHelper *> next;
    ShiftHelper<ParentOp> *parent;
    ParentOp *main;
    

    void init(ParentOp * m, ShiftHelper<ParentOp> *p, void *rv){
        assert(sizeof(RcObject) <= ALIGNLEN);

        rvalue=rv;
        main=m;
        parent=p;
        next=NULL;
        type = dt_shifthelper;
    };

    ShiftHelper(ParentOp * m, ShiftHelper<ParentOp> *p, void *rv ){
        init(m, p, rv);
    };
  
    bool complete_phase1(WFVector *vec, int pos);
    bool complete(WFVector *vec, int pos);
    bool tryFree();

    bool watch(void *p, ArrayElement *a){
        
        rc_count.fetch_add(1);

        if (a->load() != p){
            rc_count.fetch_add(-1);
            return false;
        }

        main->rc_count.fetch_add(1);
        if (a->load() != p){
            rc_count.fetch_add(-1);
            main->rc_count.fetch_add(-1);
            return false;
        }

        if (main->controlOp){
            main->controlOp->rc_count.fetch_add(1);
            if (a->load() != p){ 
                main->controlOp->rc_count.fetch_add(-1);
                rc_count.fetch_add(-1);
                main->rc_count.fetch_add(-1);
                return false;
            }
        }
        
        return true;
    };

    void unwatch(){
        rc_count.fetch_add(-1);
        main->rc_count.fetch_add(-1);
        if (main->controlOp){
            main->controlOp->rc_count.fetch_add(-1);
        }
    };
    bool isWatched(){
        return (rc_count.load() !=0);
    };

    void * readThrough(){
    	if(main->isComplete.load()){
    		return main->readThrough(this);
    	}
    	else{
    		return rvalue;
    	}
    };


     
};


template<class ShiftType>
bool completeShift(ShiftType *op, WFVector *vec, int pos);


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


class EraseAt : public OpRecord{
public:
    void *result;

    int pos;
    std::atomic<ShiftHelper<EraseAt> *> next;
    std::atomic<bool> isComplete;
    
	void init(int p){
        assert(sizeof(RcObject) <= ALIGNLEN);

        pos=p;
        next=NULL; 
        isComplete=false;
        result=NOT_VALUE;

        type = dt_eraseAt;
    };

    EraseAt(int p){
        init(p);
    };
    
    void execute(WFVector *vec){
        s_execute(vec);
    }
    bool s_execute(WFVector *vec){
        fcount=0;
        controlWord=(std::atomic<void *> *)&(this->isComplete);

        complete(vec, pos);

        if (next.load() != (ShiftHelper<EraseAt> *)(0x1) ){
            result= next.load()->rvalue;
            return true;
        }
        else {
            result=NOT_VALUE; //throw out of bounds exception.
            return false;
        }
    };

    void * getResult(){
        return result;
    };
    
    void cleanup(WFVector *vec, int pos);
    bool complete(WFVector *vec, int pos){
        bool res= completeShift<EraseAt>(this, vec, pos);
        vec->resetMyOp();
        return res;
    };

    void *readThrough(ShiftHelper<EraseAt> *helper);

    
    bool tryFree();
    
};

void *EraseAt::readThrough(ShiftHelper<EraseAt> *helper){
	ShiftHelper<EraseAt> *parent=helper->parent;
	ShiftHelper<EraseAt> *temp;

	if(parent == NULL){
		temp=next.load();
	}
	else{
		temp=parent->next.load();
	}
	assert(temp != NULL);//because op is done
	if(temp != helper){
		return helper->rvalue;
	}
	else if(helper->rvalue == NOT_VALUE){
		return NOT_VALUE;
	}
	else{
		return helper->next.load()->rvalue;
	}
};

#include "wfv_class_fun.hpp"



#endif