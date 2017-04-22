CC = xelatex
FLAG = -shell-escape
TEX = relatorio.tex

all:
	$(CC) $(FLAG) $(TEX)
	$(CC) $(TEX)
	$(MAKE) clean
clean:
	rm --verbose *.{aux,log,mintedcmd,out,pyg,toc}
