from __future__ import print_function
import subprocess
import sys
import os
import glob

def getHeader():
  return "Test Length, Capacity, Prefill, Rates[Find Insert Update Remove], Threads, Algorithm, -Total-, Finds, Inserts, Updates, Removes, Total, -Per Thread-, Finds, Inserts, Updates, Removes, Total, VmSize"
class ResultObj:
  def __init__(self):
    value = 0
    self.algorithm = value

    self.test_length = value
    self.prefill = value
    self.rates_find = value
    self.rates_insert = value
    self.rates_update = value
    self.rates_remove = value
    self.capacity = 1024

    self.threads = value
    self.result_find = value
    self.result_insert = value
    self.result_update = value
    self.result_remove = value
    self.rep_count = 1

    self.avg_thread_result_find = value
    self.avg_thread_result_insert = value
    self.avg_thread_result_update = value
    self.avg_thread_result_remove = value
    self.avg_thread_vmsize = value
    self.avg_thread_count = value




  def toCVS(self):
    total_ops = ((self.result_find/self.rep_count) + (self.result_insert/self.rep_count) + (self.result_update/self.rep_count) + (self.result_remove/self.rep_count))
    res = "%d, %d, %d, [%d %d %d %d], %d, %s, , " %(self.test_length, self.capacity, self.prefill, self.rates_find, self.rates_insert, self.rates_update, self.rates_remove, self.threads,  self.algorithm)
    res += "%d, %d, %d, %d, %d, , " %( (self.result_find/self.rep_count), (self.result_insert/self.rep_count), (self.result_update/self.rep_count), (self.result_remove/self.rep_count), total_ops )
    res += "%d, %d, %d, %d, " %( (self.avg_thread_result_find / self.avg_thread_count), (self.avg_thread_result_insert / self.avg_thread_count), (self.avg_thread_result_update / self.avg_thread_count), (self.avg_thread_result_remove / self.avg_thread_count) )
    res += "%d, %d" % (((self.avg_thread_result_find/ self.avg_thread_count) + (self.avg_thread_result_insert/ self.avg_thread_count) + (self.avg_thread_result_update/ self.avg_thread_count) + (self.avg_thread_result_remove/ self.avg_thread_count) ), (self.avg_thread_vmsize/ self.avg_thread_count))
    return res

  def testname(self):
    fname = self.filename()
    fname += "Alg_%s_Threads_%d" %(self.algorithm, self.threads)
    return fname

  def filename(self):
    fname = "Time_%d_Cap_%d_Pref_%d_F_%d_I_%d_U_%d_R_%d" %(self.test_length,
      self.capacity, self.prefill, self.rates_find, self.rates_insert,
      self.rates_update, self.rates_remove)
    return fname

  def merge(self, other):
    self.result_find += other.result_find
    self.result_insert += other.result_insert
    self.result_update += other.result_update
    self.result_remove += other.result_remove
    self.rep_count += 1

    self.avg_thread_result_find += other.avg_thread_result_find
    self.avg_thread_result_insert += other.avg_thread_result_insert
    self.avg_thread_result_update += other.avg_thread_result_update
    self.avg_thread_result_remove += other.avg_thread_result_remove
    self.avg_thread_vmsize += other.avg_thread_vmsize

def compare(self, other):
  if (self.test_length != other.test_length) :
    return (self.test_length < other.test_length)
  elif (self.prefill != other.prefill) :
    return (self.prefill < other.prefill)
  elif (self.capacity != other.capacity) :
    return (self.capacity < other.capacity)
  elif (self.rates_find != other.rates_find) :
    return (self.rates_find < other.rates_find)
  elif (self.rates_insert != other.rates_insert) :
    return (self.rates_insert < other.rates_insert)
  elif (self.rates_update != other.rates_update) :
    return (self.rates_update < other.rates_update)
  elif (self.rates_remove != other.rates_remove) :
    return (self.rates_remove < other.rates_remove)
  elif (self.threads != other.threads) :
    return (self.threads < other.threads)
  else: # (self.algorithm != other.algorithm) :
    return (self.algorithm < other.algorithm)


def get_thread_results(file_name, i, result_obj):
  temp = "Thread " + str(i)

  with open(file_name) as fin:
    while True:
      c = fin.readline()
      if c is None:
        return None
      elif temp in c.strip():
        break

    while True:
      c = fin.readline()
      if c is None:
        break
      words = c.strip().split(' ')
      value = words[len(words)-1]

      if "Find Count" in c:
        result_obj.avg_thread_result_find += int(value)
      elif "Insert Count" in c:
        result_obj.avg_thread_result_insert += int(value)
      elif "Update Count" in c:
        result_obj.avg_thread_result_update = int(value)
      elif "Remove Count" in c:
        result_obj.avg_thread_result_remove = int(value)
      elif "VmSize" in c:
        value = words[len(words)-2]
        result_obj.avg_thread_vmsize = int(value)
      elif "-fin-" in c:
        return
      else:
        # print ("Unknown: " + c)
        pass

def get_global_results(file_name):
  test_data = ResultObj()

  value = file_name[file_name.index("-")+1 : file_name.index(".")]
  test_data.rep_num = int(value)

  with open(file_name) as fin:
    while True:
      c = fin.readline()
      if c is None:
        break

      c = c.strip()
      words = c.split(' ')
      value = words[len(words)-1]
      if  "Algorithm" in c:
        value = c[c.index("=")+2:].strip()
        test_data.algorithm =  value
      elif "Number of Threads" in c:
        test_data.threads = int(value)
      elif "Execution Time" in c:
        test_data.test_length = int(value)
      elif "Prefill %" in c:
        test_data.prefill = int(value)
      elif "Find Rate" in c:
        test_data.rates_find = int(value)
      elif "Insert Rate" in c:
        test_data.rates_insert = int(value)
      elif "Update Rate" in c:
        test_data.rates_update = int(value)
      elif "Remove Rate" in c:
        test_data.rates_remove = int(value)
      elif "Find Count" in c:
        test_data.result_find = int(value)
      elif "Insert Count" in c:
        test_data.result_insert = int(value)
      elif "Update Count" in c:
        test_data.result_update = int(value)
      elif "Remove Count" in c:
        test_data.result_remove = int(value)
      elif "-fin-" in c:
        break
      elif "Exception" in c:
        return None
      else:
        # print ("Unknown: " + c)
        pass


  for x in xrange(0, int(test_data.threads)):
    get_thread_results(file_name, x, test_data)
    test_data.avg_thread_count += 1
  return test_data

if (len(sys.argv) < 2):
  print ("Error: missing directory argument.")
  exit()

directory = sys.argv[1]

if not os.path.exists(directory):
  print ("Error: Specified directory does not exist (" + directory + ").")
  exit()

files = glob.glob(directory + "*.log")
results = {}
print ("Processing files.")
for file_name in files:
#  print ("Processing ", file_name)

  test_data = get_global_results(file_name)
  if test_data is None:
    continue
  else:
    if test_data.filename() not in results:
      results[test_data.filename()] = {}
    if test_data.testname() not in results[test_data.filename()]:
      results[test_data.filename()][test_data.testname()]  = test_data
    else:
      (results[test_data.filename()][test_data.testname()]).merge(test_data)
    #print (test_data.toCVS())

print ("Done processing files.")

while (directory[len(directory)-1] == "/"):
  directory = directory[0:len(directory)-1]

with open(directory+".csv", 'w') as fout:
  fout.write(getHeader()+"\n")
  for test in results:
     for t in results[test]:
        fout.write(results[test][t].toCVS() + "\n")




