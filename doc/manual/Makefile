SHELL := /bin/bash

default: doc.pdf

%.pdf: %.tex $(wildcard inhalt/* appendix/* img/*)
	pdflatex $<
	pdflatex $<

clean:
	rm -rf *.{aux,log,toc,lof,lot,lol,thm,out,bbl,blg,dvi,ps,pdf}
