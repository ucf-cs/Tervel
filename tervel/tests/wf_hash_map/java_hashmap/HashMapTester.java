import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.Random;

import org.cliffc.high_scale_lib.NonBlockingHashMapLong;



public class HashMapTester{
  public int capacity_, num_threads_, execution_time_, prefill_;
  public int insert_rate_, find_rate_, update_rate_, remove_rate_;

  public AtomicInteger aicount = new AtomicInteger(0), aucount = new AtomicInteger(0), arcount = new AtomicInteger(0), afcount = new AtomicInteger(0);


  public AtomicInteger ready_count_ = new AtomicInteger(0);
  public AtomicBoolean running_ = new AtomicBoolean(true);
  public AtomicBoolean wait_flag_= new AtomicBoolean(true);

  public NonBlockingHashMapLong<Long> test_class_;

    public HashMapTester(final String []args) throws InterruptedException, IOException {
      int pos = 0;
    String temp = args[pos++];
    capacity_ = Integer.parseInt(temp);
    temp = args[pos++];
    num_threads_ = Integer.parseInt(temp);
    temp = args[pos++];
    execution_time_ = Integer.parseInt(temp);
    temp = args[pos++];
    prefill_ = Integer.parseInt(temp);
    temp = args[pos++];

    insert_rate_ = Integer.parseInt(temp);
    temp = args[pos++];
    find_rate_ = Integer.parseInt(temp);
    temp = args[pos++];
    update_rate_ = Integer.parseInt(temp);
    temp = args[pos++];
    remove_rate_ = Integer.parseInt(temp);


      print_test_info();

      test_class_ = new NonBlockingHashMapLong<Long>(capacity_);


      {
        Random rand = new Random(-1);
        int limit = (int)((float)(prefill_)/100.0 * capacity_);
        for (int i = 0; i < limit; i++) {
        Long key = new Long(rand.nextInt()+1);
          test_class_.put(key, key);
        }

      }

      HashMapThread[] threads = new HashMapThread[num_threads_];
      for (int i = 0; i < threads.length; i++) {
          threads[i] = new HashMapThread(i);
          threads[i].start();
      }

      while (ready_count_.get() < num_threads_) {}
      ready_count_.set(0);

      wait_flag_.set(false);


      Thread.sleep(1000*execution_time_);

      running_.set(false);

      Thread.sleep(1000*1);


      while (ready_count_.get() < num_threads_) {}
      print_results();
      ready_count_.set(0);

      for (int i = 0; i < threads.length; i++) {
        threads[i].join();
      }
  }
  public static void main(final String []args) throws InterruptedException, IOException {
    new HashMapTester(args);

  }


  void print_test_info() {
    System.out.println("Test Results:" + "\n"
        + "\tAlgorithm = " + "CliffClick-Java"+ "\n"
        + "\tNumber of Threads = " + num_threads_ + "\n"
        + "\tExecution Time = " + execution_time_ + "\n"
        + "\tPrefill % = " + prefill_ + "\n"
        + "\tFind Rate = " + find_rate_ + "\n"
        + "\tInsert Rate = " + insert_rate_ + "\n"
        + "\tUpdate Rate = " + update_rate_ + "\n"
        + "\tRemove Rate = " + remove_rate_ + "\n"
        );
    }

  void print_results() throws IOException, InterruptedException {
    System.out.println("\t  Find Count = " + afcount.get() + "\n"
        + "\tInsert Count = " + aicount.get() + "\n"
        + "\tUpdate Count = " + aucount.get() + "\n"
        + "\tRemove Count = " + arcount.get() + "\n"
        + "-------------------/proc/meminfo-----------------------" + "\n"
        );

      system("cat /proc/meminfo");
      System.out.println( "-------------------fin-----------------------"
        + "\n" );
    }

  void thread_print_results(int thread_id, int fcount, int icount,
      int ucount, int rcount) throws IOException, InterruptedException {
    System.out.println( "Thread " + thread_id + "\n"
        + "\t  Find Count = " + fcount + "\n"
        + "\tInsert Count = " + icount + "\n"
        + "\tUpdate Count = " + ucount + "\n"
        + "\tRemove Count = " + rcount + "\n"
         + "-------------------/proc/self/status-----------------------" + "\n"
        );

      system("cat /proc/self/status");
      System.out.println( "-------------------fin-----------------------"
        + "\n" + "\n" );
    }

    void update_results(int fcount, int icount, int ucount,
      int rcount) {
      afcount.addAndGet(fcount);
      aicount.addAndGet(icount);
      aucount.addAndGet(ucount);
      arcount.addAndGet(rcount);
    };


    static void system(String cmd) throws IOException, InterruptedException {
      Runtime r = Runtime.getRuntime();
      Process p = r.exec(cmd);
      p.waitFor();
      BufferedReader b = new BufferedReader(new InputStreamReader(p.getInputStream()));
      String line = "";

      while ((line = b.readLine()) != null) {
        System.out.println(line);
      }

      b.close();
    }

  public class HashMapThread extends Thread {
  int thread_id_ = 1;
    public HashMapThread(int thread_id) {
      thread_id_ = thread_id;
    }

    public void run() {
      Random rand = new Random(thread_id_);

      //boost::uniform_int<> brandValues(2, std::numeric_limits<int>::max());
      // boost::uniform_int<> brandOperations(1, 100);

    final int frate = find_rate_;
    final int irate = frate + insert_rate_;
    final int urate = irate + update_rate_;
    final int rrate = urate + remove_rate_;

    int fcount = 0;
    int icount = 0;
    int ucount = 0;
    int rcount = 0;

    assert(rrate == 100);


    ready_count_.addAndGet(1);
    while (wait_flag_.get()) {};

    while (running_.get()) {
    int op = rand.nextInt(101);

    Long key = new Long(rand.nextInt()+2);

      if (op < frate) {
        long temp = -1;
        test_class_.get(temp);
        fcount++;
      } else if (op < irate) {
        test_class_.putIfAbsent(key, key);
        icount++;
      } else if (op < urate) {

        Long new_value = test_class_.get(key);
        if (new_value != null) {
          new_value += 1000;
          test_class_.put(key, new_value);
        }
        // fcount++;
        ucount++;
      } else if (op < rrate) {
        test_class_.remove(key);
        rcount++;
      } else {
        assert(false);
      }
    }  // end while running


    update_results(fcount, icount, ucount, rcount);
    ready_count_.addAndGet(1);

    while (ready_count_.get() < num_threads_) {};

    while (ready_count_.get() != thread_id_) {};

    try {
    thread_print_results(thread_id_, fcount, icount, ucount, rcount);
    } catch (IOException e) {
    e.printStackTrace();
    } catch (InterruptedException e) {
      e.printStackTrace();
    }
    ready_count_.addAndGet(1);
    }  // run
  }  // hash map thread
}  // HashMapTest clas
