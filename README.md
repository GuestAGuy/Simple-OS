
data structures:
- (int) clock time: init to 0, starts when first CPU job starts
    

- (struct) CPU info:
--- (int) CPU status (idle = 0, busy = 1)
--- (float) stop time (time to stop running curr process)
-- stats: 
--- (float) total time spent busy
--- (float) total time spent idle
--- (float) CPU utilization
--- (int) number of dispatches
--- (float) overall throughput

- (struct) I/O info: same as CPU info?
--- (int) I/O status (idle = 0, busy = 1)
--- (float) stop time (time to stop running curr process)
-- stats: 
--- (float) total time spent busy
--- (float) total time spent idle
--- (float) I/O device utilization
--- (int) number of times I/O was started
--- (float) overall throughput

- (Process []) process array: init empty, read each row of file
                              directly into Process object
- (Char [] [])  process states: ["ready", "cpu running", "io_block", "io/running", "done"]
                              this is a state machine because the transitions from one state
                              into another are always the same

- (struct) Process info:
--- (string) process name
--- (float) inital CPU time
--- (float) remaining CPU time (until finished)
--- (float) remaining CPU time (until next block)
--- (int) number of time process was throw into CPU
--- (int) number of time process throw into I/O
--- (float) time process spent in I/O
--- (float) wall clock time at finish
--- (int) index of current state from state array


====SOME HINTS=====
Data structures:

An integer variable to record the current (wall clock) time; initially it is 0.
A structure to record information about the CPU. This includes the relevant statistics,
    the CPU status (busy or idle), and a time field that gives the time at which the currently
    running job should stop running. Of course, that field is relevant only when the CPU status is busy!
A structure to record information about the I/O device. The structure is analogous to that for the CPU.
A structure to store information about each job. This consists of the job’s name, its priority, its
    probability of blocking,
    the time it is to run, and how much run time remains. There is also space for two link fields.
A linked list for the ready queue and another for the I/O queue. These consist of structures for the jobs
    linked together.

===STATS===

Statistics To Gather
As you perform the simulation, gather the following statistics.

    For Each Process
Its name;
Its total CPU time (as read);
The wall clock time at which it was completed (this is the value of your own counter, not the time of day);
How many times the process was given the CPU;
How many times the process blocked for I/O; and
How much time the process spent doing I/O.
    For the System
The wall clock time at which the simulation finished.
    For the CPU
Total time spent busy;
Total time spent idle;
CPU utilization (= busy time / total time);
Number of dispatches (number of times a process is moved onto the CPU); and
Overall throughput (= number of processes / total time).
    For the I/O device
Total time spent busy;
Total time spent idle;
I/O device utilization (= busy time / total time);
Number of times I/O was started; and
Overall throughput (= number of jobs / total time).

```
===OUTPUT===
Processes:
Name	CPU time	When done	# Dispatches	# Blocked for I/O	I/O time
name	n	n	n	n	n
…
System:
The wall clock time at which the simulation finished: n
CPU:
Total time spent busy: n
Total time spent idle: n
CPU utilization: nn.nn
Number of dispatches: n
Overall throughput: nn.nn
I/O device:
Total time spent busy: n
Total time spent idle: n
I/O device utilization: nn.nn
Number of times I/O was started: n
Overall throughput: nn.nn
```

State machine (?):

```
                   Process (FCFS)
                    ┌───────────────┐
                    │               │
          ┌─────────►   CPU running ├──┐
          │         │               │  │
          │         └─────┬─────────┘  │
┌─────────┴────────┐      │            │
│  Ready queue     │      │            │
│                  │      │            │
│     START STATE  │      │            │
└─────────▲────────┘      │            │
          │             ┌─▼────┐       │
          │             │      │       │
          │             │ Done │  ┌────▼───────┐
          │             │      │  │ I/O Block  │
          │             └──────┘  │            │
          │                       │  queue     │
          │                       └──────┬─────┘
          │                              │
          │         ┌─────────────────┐  │
          │         │                 │  │
          └─────────┤  I/O running    │◄─┘
                    └─────────────────┘
```

state transition conditions:
- (ready->running): + reaches head of ready queue, cpu is free, block probability calc is complete
                    + it will go to running state regardless of whether we'll eventually block
                      in otherwords, (process.toBlock == false) or (process.timeUntilBlock>0)
- (running->block): + process.toBlock is true, process.timeUntilBlock==0
- (block->io):      + block queue is empty, iotime calc is complete
- (io->ready):      + process.ioTime == 0

- (ready->done):    + (process.elapsedCPUTime == process.initialCPUTime), the process is removed
                      from both queues
