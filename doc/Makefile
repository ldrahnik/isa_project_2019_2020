file=manual

doc:
	latex ${file}.tex
	bibtex ${file}
	latex ${file}.tex
	latex ${file}.tex
	dvips ${file}.dvi
	ps2pdf -sPAPERSIZE=a4 ${file}.ps

clean:
	rm -f *.dvi
	rm -f *.aux
	rm -f *.ps
	rm -f *.bbl
	rm -f *.blg
	rm -f *.pdf
