#define HEADER_H

//Arrays to be used
int syscallNumbers[] = {43,288,21,163,248,159,183,37,158,49,321,12,451,125,126
                                    ,80,90,92,161,305,229,228,230,227,56,435,3,436,42,326,85,
                                    174,176,32,33,292,213,291,233,214,281,441,232,215,284,290,
                                    59,322,60,231,269,439,221,285,300,301,81,91,268,452,93,260,
                                    72,75,193,313,196,73,57,199,431,190,432,430,433,5,138,74,
                                    77,202,456,455,449,454,261,177,239,274,211,309,79,78,217,108,
                                    107,104,115,36,52,121,111,39,181,110,140,318,120,118,97,98,
                                    124,51,55,186,96,102,191,175,254,253,294,255,210,207,208,333,
                                    206,209,426,427,425,16,173,172,252,251,312,320,246,250,62,
                                    445,444,446,94,192,86,265,50,458,194,195,212,198,8,189,459,
                                    461,460,6,28,453,237,324,319,447,256,27,83,258,133,259,149,
                                    325,151,9,154,165,442,429,279,10,245,244,240,243,242,241,25,
                                    71,68,70,69,26,150,152,11,303,35,262,180,2,304,428,257,437,
                                    34,298,135,438,434,424,22,293,155,330,331,329,7,271,157,17,
                                    295,327,302,440,448,310,311,270,101,182,18,296,328,178,179,
                                    443,0,187,89,267,19,169,45,299,47,216,197,82,264,316,249,
                                    219,84,334,13,127,14,129,15,130,128,297,146,147,204,315,143,
                                    145,148,203,314,142,144,24,317,185,23,66,64,65,220,40,307,
                                    46,44,238,450,273,205,218,171,123,122,106,116,170,38,308,109,
                                    141,114,119,117,113,160,112,54,164,105,188,30,31,67,29,48,
                                    131,282,289,41,53,275,4,137,457,332,168,167,88,266,162,277,
                                    306,156,139,99,103,276,234,201,222,226,225,224,223,283,287,
                                    286,100,200,76,184,95,166,63,87,263,272,134,323,136,132,280,
                                    235,58,153,278,236,61,247,1,20};

char *syscallNames[] = {"accept","accept4","access","acct","add_key","adjtimex","afs_syscall","alarm","arch_prctl","bind","bpf",
                    "brk","cachestat","capget","capset","chdir","chmod","chown","chroot","clock_adjtime","clock_getres",
                    "clock_gettime","clock_nanosleep","clock_settime","clone","clone3","close","close_range","connect",
                    "copy_file_range","creat","create_module","delete_module","dup","dup2","dup3","epoll_create","epoll_create1",
                    "epoll_ctl","epoll_ctl_old","epoll_pwait","epoll_pwait2","epoll_wait","epoll_wait_old","eventfd","eventfd2",
                    "execve","execveat","exit","exit_group","faccessat","faccessat2","fadvise64","fallocate","fanotify_init",
                    "fanotify_mark","fchdir","fchmod","fchmodat","fchmodat2","fchown","fchownat","fcntl","fdatasync","fgetxattr",
                    "finit_module","flistxattr","flock","fork","fremovexattr","fsconfig","fsetxattr","fsmount","fsopen","fspick",
                    "fstat","fstatfs","fsync","ftruncate","futex","futex_requeue","futex_wait","futex_waitv","futex_wake","futimesat",
                    "get_kernel_syms","get_mempolicy","get_robust_list","get_thread_area","getcpu","getcwd","getdents","getdents64",
                    "getegid","geteuid","getgid","getgroups","getitimer","getpeername","getpgid","getpgrp","getpid","getpmsg","getppid",
                    "getpriority","getrandom","getresgid","getresuid","getrlimit","getrusage","getsid","getsockname","getsockopt",
                    "gettid","gettimeofday","getuid","getxattr","init_module","inotify_add_watch","inotify_init","inotify_init1",
                    "inotify_rm_watch","io_cancel","io_destroy","io_getevents","io_pgetevents","io_setup","io_submit","io_uring_enter",
                    "io_uring_register","io_uring_setup","ioctl","ioperm","iopl","ioprio_get","ioprio_set","kcmp","kexec_file_load",
                    "kexec_load","keyctl","kill","landlock_add_rule","landlock_create_ruleset","landlock_restrict_self","lchown",
                    "lgetxattr","link","linkat","listen","listmount","listxattr","llistxattr","lookup_dcookie","lremovexattr","lseek",
                    "lsetxattr","lsm_get_self_attr","lsm_list_modules","lsm_set_self_attr","lstat","madvise","map_shadow_stack",
                    "mbind","membarrier","memfd_create","memfd_secret","migrate_pages","mincore","mkdir","mkdirat","mknod","mknodat",
                    "mlock","mlock2","mlockall","mmap","modify_ldt","mount","mount_setattr","move_mount","move_pages","mprotect",
                    "mq_getsetattr","mq_notify","mq_open","mq_timedreceive","mq_timedsend","mq_unlink","mremap","msgctl","msgget",
                    "msgrcv","msgsnd","msync","munlock","munlockall","munmap","name_to_handle_at","nanosleep","newfstatat","nfsservctl",
                    "open","open_by_handle_at","open_tree","openat","openat2","pause","perf_event_open","personality","pidfd_getfd",
                    "pidfd_open","pidfd_send_signal","pipe","pipe2","pivot_root","pkey_alloc","pkey_free","pkey_mprotect","poll",
                    "ppoll","prctl","pread64","preadv","preadv2","prlimit64","process_madvise","process_mrelease","process_vm_readv",
                    "process_vm_writev","pselect6","ptrace","putpmsg","pwrite64","pwritev","pwritev2","query_module","quotactl",
                    "quotactl_fd","read","readahead","readlink","readlinkat","readv","reboot","recvfrom","recvmmsg","recvmsg",
                    "remap_file_pages","removexattr","rename","renameat","renameat2","request_key","restart_syscall","rmdir",
                    "rseq","rt_sigaction","rt_sigpending","rt_sigprocmask","rt_sigqueueinfo","rt_sigreturn","rt_sigsuspend",
                    "rt_sigtimedwait","rt_tgsigqueueinfo","sched_get_priority_max","sched_get_priority_min","sched_getaffinity",
                    "sched_getattr","sched_getparam","sched_getscheduler","sched_rr_get_interval","sched_setaffinity","sched_setattr",
                    "sched_setparam","sched_setscheduler","sched_yield","seccomp","security","select","semctl","semget","semop",
                    "semtimedop","sendfile","sendmmsg","sendmsg","sendto","set_mempolicy","set_mempolicy_home_node","set_robust_list",
                    "set_thread_area","set_tid_address","setdomainname","setfsgid","setfsuid","setgid","setgroups","sethostname",
                    "setitimer","setns","setpgid","setpriority","setregid","setresgid","setresuid","setreuid","setrlimit","setsid",
                    "setsockopt","settimeofday","setuid","setxattr","shmat","shmctl","shmdt","shmget","shutdown","sigaltstack",
                    "signalfd","signalfd4","socket","socketpair","splice","stat","statfs","statmount","statx","swapoff","swapon",
                    "symlink","symlinkat","sync","sync_file_range","syncfs","sysctl","sysfs","sysinfo","syslog","tee","tgkill",
                    "time","timer_create","timer_delete","timer_getoverrun","timer_gettime","timer_settime","timerfd_create",
                    "timerfd_gettime","timerfd_settime","times","tkill","truncate","tuxcall","umask","umount2","uname","unlink",
                    "unlinkat","unshare","uselib","userfaultfd","ustat","utime","utimensat","utimes","vfork","vhangup","vmsplice",
                    "vserver","wait4","waitid","write","writev"};