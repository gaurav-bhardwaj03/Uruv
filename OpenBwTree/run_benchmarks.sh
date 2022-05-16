cmake -S ./
make

for threads in 1 20 40 60 80
do
  iter=0
  while [ $iter -lt 10 ]
  do
      ./build/OpenBWTree $threads 50 30 20 0 1024 100000000 500000000 80
      iter=`expr $iter + 1`
  done
done

for threads in 1 20 40 60 80
do
  iter=0
  while [ $iter -lt 10 ]
  do
      ./build/OpenBWTree $threads 95 3 2 0 1024 100000000 500000000 80
      iter=`expr $iter + 1`
  done
done

for threads in 1 20 40 60 80
do
  iter=0
  while [ $iter -lt 10 ]
  do
      ./build/OpenBWTree $threads 100 0 0 0 1024 100000000 500000000 80
      iter=`expr $iter + 1`
  done
done

for threads in 1 20 40 60 80
do
  iter=0
  while [ $iter -lt 10 ]
  do
      ./build/OpenBWTree $threads 0 100 0 0 1024 100000000 500000000 80
      iter=`expr $iter + 1`
  done
done