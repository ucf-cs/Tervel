#ifndef __SLOWPATH__
#define  __SLOWPATH__ 1

#include "node.h"

#include "wf_ring_buffer.hpp"
#include "wf_macros.hpp"


enum DescriptorType {dt_enqueue_op, dt_enqueue_helper,
                     dt_dequeue_op, dt_dequeue_helper,
                     dt_unknown};


class RcObject;
class OpRecord; 
__thread RcObject * tl_safe_pool=NULL;
__thread RcObject * tl_unsafe_pool=NULL;



class RcObject: public Node{
public:
	DescriptorType type=dt_unknown;

	std::atomic<long> rc_count;
	std::atomic<RcObject *> rc_next;

#ifdef DEBUG
    std::atomic<long> allcount;
    std::atomic<long> freecount;
#endif

    void init(){
        assert(sizeof(RcObject) <= ALIGNLEN);
		rc_count=0;
		rc_next=NULL;
        
        #ifdef DEBUG
            allcount=1;
            freecount=0;
        #endif
	};
};

class Helper: public RcObject{
public:
    //TODO in isHelper add resize bit check as well...

    static bool isHelper(void *p){
        return ( (((long)p) & 0x1)== 0x1 );
    };
    

};

class OpRecord: public RcObject{
public:
    virtual void execute(WFRingBuffer *buffer){ printf("foo" );};
    //virtual bool tryFree(){ assert(false); return false; };
    virtual Node * getResult()=0;// {assert(false); return NULL;};
};

class EnqueueHelper;
class DequeueHelper;

class EnqueueOp:public OpRecord{
public:
    std::atomic<EnqueueHelper *> assoc;

    long value;
    EnqueueOp(long v){
        assert(sizeof(EnqueueOp) <= ALIGNLEN);
        type=dt_enqueue_op;
        value=v;
        assoc.store(NULL);
    }

    void execute(WFRingBuffer *buffer) { }

    //bool tryFree() {}

    Node * getResult() {
        //assert (assoc.load() != NULL);
        //if (assoc.load() == (EnqueueHelper *) 0x1)
          //  return NULL;
        return (Node *)assoc.load();
    }
};

class DequeueOp:public OpRecord{
public:
    std::atomic<DequeueHelper *> assoc;
    
    DequeueOp(){
        assert(sizeof(DequeueOp) <= ALIGNLEN);
        type=dt_dequeue_op;
        assoc.store(NULL);
    }

    virtual void execute(WFRingBuffer *buffer) { }
   
    // bool tryFree() { }

    Node* getResult() {
        //assert (assoc.load() != NULL);
        //node = (Node *) assoc.load();
        return (Node *) assoc.load();
        //if (node == (Node *) 0x1)
          //  return false;
        //return true;
    }
};



class EnqueueHelper: public Helper{
public:
    void *value;
    std::atomic<long> pos;
    EnqueueOp *controlOp;
    void init(void *v, EnqueueOp *op){
        assert(sizeof(EnqueueHelper) <= ALIGNLEN);
        value=v;
        pos=-1;
        type=dt_enqueue_helper;
        controlOp=op;
    };

    EnqueueHelper(void *v){
        init(v, NULL);
    };

    EnqueueHelper(void *v, EnqueueOp *op){
        init(v,op);
    };

    
    
    //bool complete(WFRingBuffer *buffer, int pos);
    //bool tryFree();
    /*
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
    };*/

};


class DequeueHelper: public Helper{
public:
    std::atomic<void *> child;
    DequeueOp *controlOp;
    void init(DequeueOp *cop){
        assert(sizeof(DequeueHelper) <= ALIGNLEN);
    	child=NULL;
        type=dt_dequeue_helper;
        controlOp=cop;
    }
    DequeueHelper(){
        init(NULL);
    };
    DequeueHelper(DequeueOp *cop){
        init(cop);
    };
    
    //bool complete(WFRingBuffer *buffer, int pos);
    
    bool getResult(void *&v);
    
    //bool tryFree();

    /*
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
    */
};

#include "slowpath.hpp"

#endif
