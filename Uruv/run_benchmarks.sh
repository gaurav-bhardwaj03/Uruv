cmake -S ./
make

for threads in 1 20 40 60 80
do
  iter=0
  while [ $iter -lt 10 ]
  do
      ./build/Uruv $threads 50 30 20 0 1024 100000000 500000000 80
      iter=`expr $iter + 1`
  done
done

for threads in 1 20 40 60 80
do
  iter=0
  while [ $iter -lt 10 ]
  do
      ./build/Uruv $threads 95 3 2 0 1024 100000000 500000000 80
      iter=`expr $iter + 1`
  done
done

for threads in 1 20 40 60 80
do
  iter=0
  while [ $iter -lt 10 ]
  do
      ./build/Uruv $threads 100 0 0 0 1024 100000000 500000000 80
      iter=`expr $iter + 1`
  done
done

for threads in 1 20 40 60 80
do
  iter=0
  while [ $iter -lt 10 ]
  do
      ./build/Uruv $threads 49 30 20 1 1024 100000000 500000000 80
      iter=`expr $iter + 1`
  done
done

for threads in 1 20 40 60 80
do
  iter=0
  while [ $iter -lt 10 ]
  do
      ./build/Uruv $threads 45 30 20 5 1024 100000000 500000000 80
      iter=`expr $iter + 1`
  done
done

for threads in 1 20 40 60 80
do
  iter=0
  while [ $iter -lt 10 ]
  do
      ./build/Uruv $threads 40 30 20 10 1024 100000000 500000000 80
      iter=`expr $iter + 1`
  done
done

for threads in 1 20 40 60 80
do
  iter=0
  while [ $iter -lt 10 ]
  do
      ./build/Uruv $threads 94 3 2 1 1024 100000000 500000000 80
      iter=`expr $iter + 1`
  done
done

for threads in 1 20 40 60 80
do
  iter=0
  while [ $iter -lt 10 ]
  do
      ./build/Uruv $threads 90 3 2 5 1024 100000000 500000000 80
      iter=`expr $iter + 1`
  done
done

for threads in 1 20 40 60 80
do
  iter=0
  while [ $iter -lt 10 ]
  do
      ./build/Uruv $threads 85 3 2 10 1024 100000000 500000000 80
      iter=`expr $iter + 1`
  done
done