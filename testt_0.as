	lw	0	1	five
start	lw	0	4	SubAdr
	lw	0	4	Stack
SubAdr	jalr	4	7
	beq	0	1	done
	beq	0	0	start
done	halt
ive	.fill	5
five	.fill	start
ffive	.fill	SubAdr
