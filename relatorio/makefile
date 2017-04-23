CC = xelatex
FLAG = -shell-escape
TEX = relatorio.tex

all:
	$(CC) $(FLAG) $(TEX)
	$(CC) $(FLAG) $(TEX)
	$(MAKE) clean
clean:
	rm --verbose *.{aux,log,out,toc}
	rm --recursive --force --verbose _minted-main _minted-relatorio
