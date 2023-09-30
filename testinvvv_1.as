	lw	0	1	fivee	load reg1 with 5 (symbolic address)
	sw	1	2	3	load reg2 with -1 (numeric address)
	lw	0	1	StAddr	load reg1 with 5 (symbolic address)
	sw	1	2	3	load reg2 with -1 (numeric address)
startt	nor	4	4	2
sstart	add	1	2	1	decrement reg1
	add	1	2	1	decrement reg1
	beq	0	1	2	goto end of program when reg1==0
	beq	0	0	sstart	go back to the beginning of the loop
	beq	2	7	dome	go back to the beginning of the loop
	lw	2	4	sstart
	sw	6	4	32764
Stack	noop
	beq	0	1	2	goto end of program when reg1==0
	beq	0	0	startt	go back to the beginning of the loop
	beq	2	7	dome	go back to the beginning of the loop
	lw	2	4	sstart
StAddr	sw	6	4	32764
	noop
	beq	0	1	2	goto end of program when reg1==0
	beq	0	0	startt	go back to the beginning of the loop
	beq	2	7	dome	go back to the beginning of the loop
	lw	2	4	sstart
	sw	6	4	32764
	noop
dome	halt				end of program
that	lw	0	3	that	this is to test
fivee	.fill	5
neg2	.fill	-1
	.fill	sstart			will contain the address of sstart (2)
