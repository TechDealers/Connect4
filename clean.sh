#!/bin/bash

# Clean up message queues
for msqid in `ipcs -q | awk '/0x/{print $2}'`
do
   ipcrm -q $msqid
done

# Clean up shared memory
for shmid in `ipcs -m | awk '/0x/{print $2}'`
do
   ipcrm -m $shmid
done

# Clean up semaphores
for semid in `ipcs -s | awk '/0x/{print $2}'`
do
   ipcrm -s $semid
done

echo "All IPCs cleaned up."