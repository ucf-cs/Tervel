dir=$(date +"%T")
mkdir $dir
mkdir $dir/wf_stack/
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/wf_stack.x -num_threads=1 -execution_time=20 -prefill=0 -main_sleep=10 1 50 50 "  30 "PAPI_TOT_INS"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/wf_stack.x -num_threads=2 -execution_time=20 -prefill=0 -main_sleep=10 2 50 50 "  30 "PAPI_TOT_INS"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/wf_stack.x -num_threads=4 -execution_time=20 -prefill=0 -main_sleep=10 4 50 50 "  30 "PAPI_TOT_INS"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/wf_stack.x -num_threads=8 -execution_time=20 -prefill=0 -main_sleep=10 8 50 50 "  30 "PAPI_TOT_INS"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/wf_stack.x -num_threads=16 -execution_time=20 -prefill=0 -main_sleep=10 16 50 50 "  30 "PAPI_TOT_INS"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/wf_stack.x -num_threads=32 -execution_time=20 -prefill=0 -main_sleep=10 32 50 50 "  30 "PAPI_TOT_INS"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/wf_stack.x -num_threads=64 -execution_time=20 -prefill=0 -main_sleep=10 64 50 50 "  30 "PAPI_TOT_INS"
mkdir $dir/wf_stack/PAPI_TOT_INS
mkdir $dir/wf_stack/PAPI_TOT_INS/data/
mv /tmp/damian/ldmstest/* $dir/wf_stack/PAPI_TOT_INS/data/
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/wf_stack.x -num_threads=1 -execution_time=20 -prefill=0 -main_sleep=10 1 50 50 "  30 "PAPI_TOT_CYC"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/wf_stack.x -num_threads=2 -execution_time=20 -prefill=0 -main_sleep=10 2 50 50 "  30 "PAPI_TOT_CYC"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/wf_stack.x -num_threads=4 -execution_time=20 -prefill=0 -main_sleep=10 4 50 50 "  30 "PAPI_TOT_CYC"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/wf_stack.x -num_threads=8 -execution_time=20 -prefill=0 -main_sleep=10 8 50 50 "  30 "PAPI_TOT_CYC"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/wf_stack.x -num_threads=16 -execution_time=20 -prefill=0 -main_sleep=10 16 50 50 "  30 "PAPI_TOT_CYC"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/wf_stack.x -num_threads=32 -execution_time=20 -prefill=0 -main_sleep=10 32 50 50 "  30 "PAPI_TOT_CYC"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/wf_stack.x -num_threads=64 -execution_time=20 -prefill=0 -main_sleep=10 64 50 50 "  30 "PAPI_TOT_CYC"
mkdir $dir/wf_stack/PAPI_TOT_CYC
mkdir $dir/wf_stack/PAPI_TOT_CYC/data/
mv /tmp/damian/ldmstest/* $dir/wf_stack/PAPI_TOT_CYC/data/
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/wf_stack.x -num_threads=1 -execution_time=20 -prefill=0 -main_sleep=10 1 50 50 "  30 "PAPI_RES_STL"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/wf_stack.x -num_threads=2 -execution_time=20 -prefill=0 -main_sleep=10 2 50 50 "  30 "PAPI_RES_STL"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/wf_stack.x -num_threads=4 -execution_time=20 -prefill=0 -main_sleep=10 4 50 50 "  30 "PAPI_RES_STL"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/wf_stack.x -num_threads=8 -execution_time=20 -prefill=0 -main_sleep=10 8 50 50 "  30 "PAPI_RES_STL"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/wf_stack.x -num_threads=16 -execution_time=20 -prefill=0 -main_sleep=10 16 50 50 "  30 "PAPI_RES_STL"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/wf_stack.x -num_threads=32 -execution_time=20 -prefill=0 -main_sleep=10 32 50 50 "  30 "PAPI_RES_STL"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/wf_stack.x -num_threads=64 -execution_time=20 -prefill=0 -main_sleep=10 64 50 50 "  30 "PAPI_RES_STL"
mkdir $dir/wf_stack/PAPI_RES_STL
mkdir $dir/wf_stack/PAPI_RES_STL/data/
mv /tmp/damian/ldmstest/* $dir/wf_stack/PAPI_RES_STL/data/
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/wf_stack.x -num_threads=1 -execution_time=20 -prefill=0 -main_sleep=10 1 50 50 "  30 "PAPI_BR_INS"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/wf_stack.x -num_threads=2 -execution_time=20 -prefill=0 -main_sleep=10 2 50 50 "  30 "PAPI_BR_INS"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/wf_stack.x -num_threads=4 -execution_time=20 -prefill=0 -main_sleep=10 4 50 50 "  30 "PAPI_BR_INS"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/wf_stack.x -num_threads=8 -execution_time=20 -prefill=0 -main_sleep=10 8 50 50 "  30 "PAPI_BR_INS"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/wf_stack.x -num_threads=16 -execution_time=20 -prefill=0 -main_sleep=10 16 50 50 "  30 "PAPI_BR_INS"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/wf_stack.x -num_threads=32 -execution_time=20 -prefill=0 -main_sleep=10 32 50 50 "  30 "PAPI_BR_INS"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/wf_stack.x -num_threads=64 -execution_time=20 -prefill=0 -main_sleep=10 64 50 50 "  30 "PAPI_BR_INS"
mkdir $dir/wf_stack/PAPI_BR_INS
mkdir $dir/wf_stack/PAPI_BR_INS/data/
mv /tmp/damian/ldmstest/* $dir/wf_stack/PAPI_BR_INS/data/
mkdir $dir/lf_stack/
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=1 -execution_time=20 -prefill=0 -main_sleep=10 1 50 50 "  30 "PAPI_TOT_INS"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=2 -execution_time=20 -prefill=0 -main_sleep=10 2 50 50 "  30 "PAPI_TOT_INS"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=4 -execution_time=20 -prefill=0 -main_sleep=10 4 50 50 "  30 "PAPI_TOT_INS"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=8 -execution_time=20 -prefill=0 -main_sleep=10 8 50 50 "  30 "PAPI_TOT_INS"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=16 -execution_time=20 -prefill=0 -main_sleep=10 16 50 50 "  30 "PAPI_TOT_INS"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=32 -execution_time=20 -prefill=0 -main_sleep=10 32 50 50 "  30 "PAPI_TOT_INS"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=64 -execution_time=20 -prefill=0 -main_sleep=10 64 50 50 "  30 "PAPI_TOT_INS"
mkdir $dir/lf_stack/PAPI_TOT_INS
mkdir $dir/lf_stack/PAPI_TOT_INS/data/
mv /tmp/damian/ldmstest/* $dir/lf_stack/PAPI_TOT_INS/data/
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=1 -execution_time=20 -prefill=0 -main_sleep=10 1 50 50 "  30 "PAPI_TOT_CYC"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=2 -execution_time=20 -prefill=0 -main_sleep=10 2 50 50 "  30 "PAPI_TOT_CYC"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=4 -execution_time=20 -prefill=0 -main_sleep=10 4 50 50 "  30 "PAPI_TOT_CYC"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=8 -execution_time=20 -prefill=0 -main_sleep=10 8 50 50 "  30 "PAPI_TOT_CYC"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=16 -execution_time=20 -prefill=0 -main_sleep=10 16 50 50 "  30 "PAPI_TOT_CYC"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=32 -execution_time=20 -prefill=0 -main_sleep=10 32 50 50 "  30 "PAPI_TOT_CYC"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=64 -execution_time=20 -prefill=0 -main_sleep=10 64 50 50 "  30 "PAPI_TOT_CYC"
mkdir $dir/lf_stack/PAPI_TOT_CYC
mkdir $dir/lf_stack/PAPI_TOT_CYC/data/
mv /tmp/damian/ldmstest/* $dir/lf_stack/PAPI_TOT_CYC/data/
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=1 -execution_time=20 -prefill=0 -main_sleep=10 1 50 50 "  30 "PAPI_RES_STL"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=2 -execution_time=20 -prefill=0 -main_sleep=10 2 50 50 "  30 "PAPI_RES_STL"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=4 -execution_time=20 -prefill=0 -main_sleep=10 4 50 50 "  30 "PAPI_RES_STL"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=8 -execution_time=20 -prefill=0 -main_sleep=10 8 50 50 "  30 "PAPI_RES_STL"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=16 -execution_time=20 -prefill=0 -main_sleep=10 16 50 50 "  30 "PAPI_RES_STL"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=32 -execution_time=20 -prefill=0 -main_sleep=10 32 50 50 "  30 "PAPI_RES_STL"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=64 -execution_time=20 -prefill=0 -main_sleep=10 64 50 50 "  30 "PAPI_RES_STL"
mkdir $dir/lf_stack/PAPI_RES_STL
mkdir $dir/lf_stack/PAPI_RES_STL/data/
mv /tmp/damian/ldmstest/* $dir/lf_stack/PAPI_RES_STL/data/
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=1 -execution_time=20 -prefill=0 -main_sleep=10 1 50 50 "  30 "PAPI_BR_INS"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=2 -execution_time=20 -prefill=0 -main_sleep=10 2 50 50 "  30 "PAPI_BR_INS"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=4 -execution_time=20 -prefill=0 -main_sleep=10 4 50 50 "  30 "PAPI_BR_INS"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=8 -execution_time=20 -prefill=0 -main_sleep=10 8 50 50 "  30 "PAPI_BR_INS"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=16 -execution_time=20 -prefill=0 -main_sleep=10 16 50 50 "  30 "PAPI_BR_INS"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=32 -execution_time=20 -prefill=0 -main_sleep=10 32 50 50 "  30 "PAPI_BR_INS"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=64 -execution_time=20 -prefill=0 -main_sleep=10 64 50 50 "  30 "PAPI_BR_INS"
mkdir $dir/lf_stack/PAPI_BR_INS
mkdir $dir/lf_stack/PAPI_BR_INS/data/
mv /tmp/damian/ldmstest/* $dir/lf_stack/PAPI_BR_INS/data/
