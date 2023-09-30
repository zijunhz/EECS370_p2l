	lw	0	1	five
	lw	0	4	SubAdr
	sw	0	4	Stack
start	jalr	4	7
done	beq	0	1	done
	beq	0	0	start
Done	halt
ive	.fill	5
five	.fill	start
ffive	.fill	SubAdr
