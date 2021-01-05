                if(tempQuantum==0||k==0){
                    k=-1;
                    tempQuantum=quantum;
                    cur_process->pid=fork();
                    int cur_pid=cur_process->pid;
                    //child block
                    if(cur_pid==0){
                        char*args[]={"./process.out",cur_process->remainingTime};
                        execv(args[0],args);
                        exit(0);
                    }
                    else if(cur_pid==-1){
                        //Error in forking.
                    }
                    //Parent block
                    else{

                    }
                }
                struct msgProcess p;
                int rec_val=msgrcv(PG2S_msqid , &p, sizeof(p.p),0, IPC_NOWAIT);
                printf("SCH::clk %d process id %d,arrival %d,runTime %d,priority %d,memsize %d,pid %ld\n",getClk()
                        ,p.p.id,p.p.arrivalTime,p.p.runTime,p.p.priority,p.p.memSize,p.p.pid);
                enqueue(RR_Queue,&p.p);
                cur_clk=prev_clk;
                tempQuantum--;
                //decrease remaining time of current process
                cur_process->remainingTime--;