dir=$(date +"%T")
mkdir $dir
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=1 -execution_time=20 -main_sleep=10 1 50 50 " 30 "PAPI_TOT_INS"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=2 -execution_time=20 -main_sleep=10 2 50 50 " 30 "PAPI_TOT_INS"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=4 -execution_time=20 -main_sleep=10 4 50 50 " 30 "PAPI_TOT_INS"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=8 -execution_time=20 -main_sleep=10 8 50 50 " 30 "PAPI_TOT_INS"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=16 -execution_time=20 -main_sleep=10 16 50 50 " 30 "PAPI_TOT_INS"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=32 -execution_time=20 -main_sleep=10 32 50 50 " 30 "PAPI_TOT_INS"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=64 -execution_time=20 -main_sleep=10 64 50 50 " 30 "PAPI_TOT_INS"
mkdir $dir/PAPI_TOT_INS
mv /tmp/damian/ldmstest/* $dir/PAPI_TOT_INS
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=1 -execution_time=20 -main_sleep=10 1 50 50 " 30 "PAPI_TOT_CYC"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=2 -execution_time=20 -main_sleep=10 2 50 50 " 30 "PAPI_TOT_CYC"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=4 -execution_time=20 -main_sleep=10 4 50 50 " 30 "PAPI_TOT_CYC"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=8 -execution_time=20 -main_sleep=10 8 50 50 " 30 "PAPI_TOT_CYC"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=16 -execution_time=20 -main_sleep=10 16 50 50 " 30 "PAPI_TOT_CYC"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=32 -execution_time=20 -main_sleep=10 32 50 50 " 30 "PAPI_TOT_CYC"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=64 -execution_time=20 -main_sleep=10 64 50 50 " 30 "PAPI_TOT_CYC"
mkdir $dir/PAPI_TOT_CYC
mv /tmp/damian/ldmstest/* $dir/PAPI_TOT_CYC
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=1 -execution_time=20 -main_sleep=10 1 50 50 " 30 "PAPI_RES_STL"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=2 -execution_time=20 -main_sleep=10 2 50 50 " 30 "PAPI_RES_STL"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=4 -execution_time=20 -main_sleep=10 4 50 50 " 30 "PAPI_RES_STL"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=8 -execution_time=20 -main_sleep=10 8 50 50 " 30 "PAPI_RES_STL"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=16 -execution_time=20 -main_sleep=10 16 50 50 " 30 "PAPI_RES_STL"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=32 -execution_time=20 -main_sleep=10 32 50 50 " 30 "PAPI_RES_STL"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=64 -execution_time=20 -main_sleep=10 64 50 50 " 30 "PAPI_RES_STL"
mkdir $dir/PAPI_RES_STL
mv /tmp/damian/ldmstest/* $dir/PAPI_RES_STL
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=1 -execution_time=20 -main_sleep=10 1 50 50 " 30 "PAPI_BR_INS"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=2 -execution_time=20 -main_sleep=10 2 50 50 " 30 "PAPI_BR_INS"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=4 -execution_time=20 -main_sleep=10 4 50 50 " 30 "PAPI_BR_INS"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=8 -execution_time=20 -main_sleep=10 8 50 50 " 30 "PAPI_BR_INS"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=16 -execution_time=20 -main_sleep=10 16 50 50 " 30 "PAPI_BR_INS"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=32 -execution_time=20 -main_sleep=10 32 50 50 " 30 "PAPI_BR_INS"
bash /home/damian/GitProjects/ovis_clean2/opt/ovis/bin/ldms_local_papi_test.sh "./Executables/lf_stack.x -num_threads=64 -execution_time=20 -main_sleep=10 64 50 50 " 30 "PAPI_BR_INS"
mkdir $dir/PAPI_BR_INS
mv /tmp/damian/ldmstest/* $dir/PAPI_BR_INS
