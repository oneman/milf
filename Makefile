compile:
	@cc -Wall milf.c -o milf
	@cc -Wall b1nd.c -o b1nd
	@gcc -Wall milf.c -o milf_gcc
	@gcc -Wall b1nd.c -o b1nd_gcc
	@clang -Wall milf.c -o milf_clang
	@clang -Wall b1nd.c -o b1nd_clang
	@./configure
