build:
	mpicc -o static_gcd static_gcd.c 
	mpicc -o dynamic_gcd dynamic_gcd.c

clean:
	rm -f *.o static_gcd dynamic_gcd