g++ -std=c++17 Expt1.cpp -lpthread -ldl

#1M, 1.5M
for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 50 30 20 0 1024 1000000 1500000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 95 3 2 0 1024 1000000 1500000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 100 0 0 0 1024 1000000 1500000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 0 100 0 0 1024 1000000 1500000
      a=`expr $a + 1`
  done
done

#100M, 150M
for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 50 30 20 0 1024 100000000 150000000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 95 3 2 0 1024 100000000 150000000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 100 0 0 0 1024 100000000 150000000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 0 100 0 0 1024 100000000 150000000
      a=`expr $a + 1`
  done
done

#1M, 500M
for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 50 30 20 0 1024 1000000 500000000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 95 3 2 0 1024 1000000 500000000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 100 0 0 0 1024 1000000 500000000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 0 100 0 0 1024 1000000 500000000
      a=`expr $a + 1`
  done
done

#100M, 500M
for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 50 30 20 0 1024 100000000 500000000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 95 3 2 0 1024 100000000 500000000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 100 0 0 0 1024 100000000 500000000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 0 100 0 0 1024 100000000 500000000
      a=`expr $a + 1`
  done
done


#1M, 1.5M
for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 49 30 20 1 1024 1000000 1500000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 45 30 20 5 1024 1000000 1500000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 40 30 20 10 1024 1000000 1500000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 94 3 2 1 1024 1000000 1500000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 90 3 2 5 1024 1000000 1500000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 85 3 2 10 1024 1000000 1500000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 100 0 0 0 1024 1000000 1500000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 0 100 0 0 1024 1000000 1500000
      a=`expr $a + 1`
  done
done

#100M, 150M
for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 49 30 20 1 1024 100000000 150000000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 45 30 20 5 1024 100000000 150000000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 40 30 20 10 1024 100000000 150000000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 94 3 2 1 1024 100000000 150000000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 90 3 2 5 1024 100000000 150000000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 85 3 2 10 1024 100000000 150000000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 100 0 0 0 1024 100000000 150000000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 0 100 0 0 1024 100000000 150000000
      a=`expr $a + 1`
  done
done

#1M, 500M
for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 49 30 20 1 1024 1000000 500000000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 45 30 20 5 1024 1000000 500000000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 40 30 20 10 1024 1000000 500000000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 94 3 2 1 1024 1000000 500000000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 90 3 2 5 1024 1000000 500000000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 85 3 2 10 1024 1000000 500000000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 100 0 0 0 1024 1000000 500000000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 0 100 0 0 1024 1000000 500000000
      a=`expr $a + 1`
  done
done

#100M, 500M
for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 49 30 20 1 1024 100000000 500000000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 45 30 20 5 1024 100000000 500000000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 40 30 20 10 1024 100000000 500000000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 94 3 2 1 1024 100000000 500000000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 90 3 2 5 1024 100000000 500000000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 85 3 2 10 1024 100000000 500000000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 100 0 0 0 1024 100000000 500000000
      a=`expr $a + 1`
  done
done

for fg_n in 1 20 40 60 80 100 120 140 160
do
  a=0
  while [ $a -lt 10 ]
  do
      ./a.out $fg_n 0 100 0 0 1024 100000000 500000000
      a=`expr $a + 1`
  done
done
