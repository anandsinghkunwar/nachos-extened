tex = report

cmp = pdflatex

ftex = $(addsuffix .tex,$(tex))

faux = $(addsuffix .aux,$(tex))

flog = $(addsuffix .log,$(tex))

all: pdf clean

pdf: 
	$(cmp) $(ftex)
clean:
	rm -f *~ $(faux) $(flog)
