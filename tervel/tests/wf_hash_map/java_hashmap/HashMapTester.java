import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Random;

import org.cliffc.high_scale_lib.NonBlockingHashMapLong;



public class HashMapTester{
  final Long GET = 0L;
  final Long INSERT = 1L;
  final Long REMOVE = 2L;
  final Long UPDATE = 3L;

  public int operations_;
  public int capacity_, num_threads_, prefill_;
  public int insert_rate_, find_rate_, update_rate_, remove_rate_;


  public AtomicInteger ready_count_ = new AtomicInteger(0);
  public AtomicBoolean running_ = new AtomicBoolean(true);
  public AtomicBoolean wait_flag_= new AtomicBoolean(true);

  public NonBlockingHashMapLong<Long> test_class_;

    public HashMapTester(final String []args) throws InterruptedException, IOException {
      int pos = 0;
      String temp = args[pos++];
      num_threads_ = Integer.parseInt(temp);
      temp = args[pos++];
      capacity_ = Integer.parseInt(temp);
      temp = args[pos++];
      prefill_ = Integer.parseInt(temp);
      temp = args[pos++];
      operations_ = Integer.parseInt(temp);


      temp = args[pos++];
      find_rate_ = Integer.parseInt(temp);
      temp = args[pos++];
      insert_rate_ = Integer.parseInt(temp) + find_rate_;
      temp = args[pos++];
      update_rate_ = Integer.parseInt(temp) + insert_rate_;
      temp = args[pos++];
      remove_rate_ = Integer.parseInt(temp) + update_rate_;

      String mem_file = args[pos++];

      String str = "[Threads " + (num_threads_) + " Capacity: " + (capacity_)+" Prefill "+ (prefill_)+
          " Operations "+ (operations_)+ "G:"+find_rate_+" U:"+update_rate_+" I:"+insert_rate_+" D:"+
          remove_rate_+" Mem Files:"+mem_file+"]";
      System.err.println(str);

      insert_rate_ += find_rate_;
      update_rate_ += insert_rate_;
      remove_rate_ += update_rate_;

      test_class_ = new NonBlockingHashMapLong<Long>(capacity_);
      HashSet<Long> keys_inserted = new HashSet<Long>();
      ArrayList<Long> valid_keys = new ArrayList<Long>();
      Triple [] assigned_op = new Triple[(operations_+prefill_)*3];

      {
        System.err.println("Generating Prefill Operations For Each Threads File");
        int ops = 0;
        Random rand = new Random(-1);
        for (int i = 0; i < prefill_; i++) {
          Long key;
          Long value = new Long(rand.nextInt()+1);
          do {
            key = new Long(rand.nextInt()+1);
          } while (keys_inserted.contains(key));
          valid_keys.add(key);
          keys_inserted.add(key);

          assigned_op[ops++] = new Triple(INSERT, key, value);
        }

        System.err.println("Generating Execution Operations For Each Threads File");
        for (int j = 0; j < operations_; j++) {
          Long op = (long) (rand.nextLong() % 100);
          Long key, value;
          value = new Long(rand.nextInt()+1);
          if (op < find_rate_) {
            op = GET;
            int pos2 = rand.nextInt() % valid_keys.size();
            key = valid_keys.get(pos2);
          } else if (op < insert_rate_) {
            op = INSERT;
            do {
              key = new Long(rand.nextInt()+1);
            } while (keys_inserted.contains(key));
            valid_keys.add(key);
            keys_inserted.add(key);

          } else if (op < update_rate_) {
            op = UPDATE;
            int pos2 = rand.nextInt() % valid_keys.size();
            key = valid_keys.get(pos2);
          } else { // op < remove_rate_) {
            op =  REMOVE;
            int pos2 = rand.nextInt() % valid_keys.size();
            key = valid_keys.get(pos2);
          }

          assigned_op[ops++] = new Triple(op, key, value);

        }

      }

      System.err.println("Creating Threads");
      HashMapThread[] threads = new HashMapThread[num_threads_];
      for (int i = 0; i < threads.length; i++) {
          threads[i] = new HashMapThread(i, num_threads_, assigned_op);
          threads[i].start();
      }

      while (ready_count_.get() < num_threads_) {}

      System.err.println("All Threads ready, singaling them to Start:");
      long start = System.nanoTime();

      wait_flag_.set(false);

      for (int i = 0; i < threads.length; i++) {
        threads[i].join();
      }
      long end = System.nanoTime();

      long measured = end - start;
      System.err.println("Threads Done thread reported " + measured);

      System.err.println("Test Completed: DS has " + test_class_.size() +" Elements");

      if(mem_file.length() > 0){
        long pid = getPID();
        String cmd = "pmap "+pid+" |grep total | awk '{print $2}' >> " + mem_file + "";

         try {
            // using the Runtime exec method:
            Process p = Runtime.getRuntime().exec(cmd);

            BufferedReader stdInput = new BufferedReader(new
                 InputStreamReader(p.getInputStream()));

            BufferedReader stdError = new BufferedReader(new
                 InputStreamReader(p.getErrorStream()));

            // read the output from the command
            System.out.println("Here is the standard output of the command:\n");
            String s;
      while ((s = stdInput.readLine()) != null) {
                System.out.println(s);
            }

            // read any errors from the attempted command
            System.out.println("Here is the standard error of the command (if any):\n");
            while ((s = stdError.readLine()) != null) {
                System.out.println(s);
            }

            System.exit(0);
        }
        catch (IOException e) {
            e.printStackTrace();
            System.exit(-1);
        }
    }

    System.err.flush();
    System.out.print(measured);
  }

  public static long getPID() {
    String processName =
      java.lang.management.ManagementFactory.getRuntimeMXBean().getName();
    return Long.parseLong(processName.split("@")[0]);
  }
  public static void main(final String []args) throws InterruptedException, IOException {
    new HashMapTester(args);

  }

  public class Triple {

    public long op;
    public long key;
    public long value;
    public Triple() {}
    public Triple(long o, long k, long v) {
      op = o;
      key = k;
      value = v;
    }
  }
  public class HashMapThread extends Thread {
    int thread_id_ = 1;
    int threads_;
    Triple [] assigned_op_;
    public HashMapThread(int thread_id, int threads, Triple [] assigned_op) {
      threads_ = threads;
      thread_id_ = thread_id;
      assigned_op_ = assigned_op;
    }

    public void run() {
      ready_count_.addAndGet(1);
      while (wait_flag_.get()) {};

      for (int i = thread_id_; i < assigned_op_.length; i += threads_) {
        long op = assigned_op_[i].op;
        long key = assigned_op_[i].key;
        long value = assigned_op_[i].value;

        if (op == INSERT) {
            test_class_.putIfAbsent(key, new Long(value));
        } else if (op == GET) {
            test_class_.get(key);
        } else if (op == UPDATE) {
            Long new_value = test_class_.get(key);
            if (new_value != null) {
              new_value += 1000;
              test_class_.put(key, new_value);
            }
        } else { // if (op == REMOVE) {
            test_class_.remove(key);
        }
      }  // for

    }  // run
  }  // hash map thread
}  // HashMapTest class
