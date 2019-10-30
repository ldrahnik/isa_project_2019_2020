doc:
	latex ${NAME}.tex
	bibtex ${NAME}
	latex ${NAME}.tex
	latex ${NAME}.tex
	dvips ${NAME}.dvi
	ps2pdf -sPAPERSIZE=a4 ${NAME}.ps

clean:
	rm -f *.dvi
	rm -f *.aux
	rm -f *.ps
	rm -f *.bbl
	rm -f *.blg
	rm -f *.pdf
