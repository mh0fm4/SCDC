# This is a Makefile!

ALL:=pdf

PNG:=plot_png image_png
PDF:=plot_pdf image_pdf tex_pdf

CLEAR:=plot_clear image_clear tex_clear
CLEAN:=plot_clean image_clean tex_clean
INFO:=plot_info image_info tex_info

DRAFT:=

PLOT_CLEAR:=
IMAGE_CLEAR:=
TEX_SUB_TEX:=
TEX_CLEAR:=
TEX_REF_BIB:=

DEPS:=
TEX_PREREQS:=
TEX_DEPS:=
GPI_DEPS:=
GPI_DAT:=
SVG_DEPS:=

TEX_MAIN_TMP_EXTS:=.log .out .nav .snm .toc .vrb .lof .loa .brf .soc
TEX_BIB_TMP_EXTS:=.aux .bbl .blg
TEX_IDX_TMP_EXTS:=.idx .ind .ilg
TEX_NOM_TMP_EXTS:=.nlo .nls
TEX_GLS_TMP_EXTS:=.glg .glo .gls .ist

MAKE_LATEX_ICONV:=
MAKE_LATEX_PREVIEW:=
MAKE_LATEX_PREVIEW_CLASS:=scrartcl
MAKE_LATEX_PREVIEW_CLASS_OPTIONS:=
MAKE_LATEX_PREVIEW_INPUTENC:=
MAKE_LATEX_PREVIEW_INPUT:=
MAKE_LATEX_PREVIEW_SIZE:=
MAKE_LATEX_PREVIEW_RULE:=
MAKE_LATEX_PREVIEW_FBOX:=

MAKE_CONVERT:=
MAKE_CONVERT_FLAGS:=

MAKE_IMAGE_EPSTOPDF:=
MAKE_IMAGE_LATEX:=

PLOT_EPS_EXTRA:=
PLOT_PDF_EXTRA:=
PLOT_CLEAR_EXTRA:=

IMAGE_EPS_EXTRA:=
IMAGE_PDF_EXTRA:=
IMAGE_CLEAR_EXTRA:=

DO_PS2EPSI:=
DO_GSBBOX:=
DO_EPSCBBOX:=


# config environment
BIBTEX:=bibtex
CAT:=cat
CONVERT:=convert
CONVERT_FLAGS:=
CP:=cp
DVIPS:=dvips
EGREP:=egrep
EPSCBBOX:=epschangebbox
EPSTOPDF:=epstopdf
GNUPLOT:=gnuplot
GNUPLOT_EXT:=gpi
GREP:=grep
GS:=gs
ICONV:=iconv
INKSCAPE:=inkscape
INKSCAPE_FLAGS:=--export-area-drawing
INKSCAPE_FLAGS_png:=
INKSCAPE_FLAGS_eps:=
INKSCAPE_FLAGS_pdf:=
INKSCAPE_FLAGS_epstex:=
INKSCAPE_FLAGS_pdftex:=
LATEX:=latex
LATEXPAND:=latexpand
MAKEIDX:=makeindex
MAKEGLS:=makeglossaries
MV:=mv
PDFLATEX:=pdflatex
PDFLATEX_FLAGS:=-halt-on-error
XELATEX:=xelatex
PDFLATEX_TMP_EXTS:=.aux .log
PDFJAM:=pdfjam
PDFTK:=pdftk
PS2EPSI:=ps2epsi
RM:=rm
RM_FLAGS:=-f
SED:=sed

TOOLS_VARS:=
TOOLS:=
#TOOLS_CHECK:=


all: _all

-include Makefile.in


TOOLS_VARS+=BIBTEX CAT CONVERT CP DVIPS EGREP EPSCBBOX EPSTOPDF GNUPLOT GREP GS ICONV INKSCAPE LATEX LATEXPAND MAKEIDX MAKEGLS MV PDFLATEX XELATEX PDFJAM PDFTK PS2EPSI RM SED
TOOLS+=$(foreach t,$(TOOLS_VARS),$($t))

listfiles = $(wildcard $(foreach d,$(2),$(if $(subst .QAWSED,,$(d)QAWSED),$(d)/)$(1)))
insubdir = $(if $(subst ./,,$(dir $(2))),cd $(dir $(2)) &&) $(1) $(notdir $(2))
ifnotempty = $(if $(2),$(1) $(2))
#ifnotempty = $(1) $(2)
ifundefined = $(if $(findstring XundefinedX,X$(origin $(1))X),$(2),$($(1)))
maketemps = $(addprefix $(basename $(1)),$(2))

# plot config
MAKE_PLOT_LATEX_ICONV?=$(MAKE_LATEX_ICONV)
MAKE_PLOT_LATEX_PREVIEW?=$(MAKE_LATEX_PREVIEW)
MAKE_PLOT_LATEX_PREVIEW_INPUTENC?=$(MAKE_LATEX_PREVIEW_INPUTENC)
MAKE_PLOT_LATEX_PREVIEW_CLASS?=$(MAKE_LATEX_PREVIEW_CLASS)
MAKE_PLOT_LATEX_PREVIEW_CLASS_OPTIONS?=$(MAKE_LATEX_PREVIEW_CLASS_OPTIONS)
MAKE_PLOT_LATEX_PREVIEW_INPUT?=$(MAKE_LATEX_PREVIEW_INPUT)
MAKE_PLOT_LATEX_PREVIEW_SIZE?=$(MAKE_LATEX_PREVIEW_SIZE)
MAKE_PLOT_LATEX_PREVIEW_RULE?=$(MAKE_LATEX_PREVIEW_RULE)
MAKE_PLOT_LATEX_PREVIEW_FBOX?=$(MAKE_LATEX_PREVIEW_FBOX)

MAKE_PLOT_CONVERT?=$(MAKE_CONVERT)
MAKE_PLOT_CONVERT_FLAGS?=$(MAKE_CONVERT_FLAGS)

DO_PLOT_PS2EPSI?=$(DO_PS2EPSI)
DO_PLOT_GSBBOX?=$(DO_GSBBOX)
DO_PLOT_EPSCBBOX?=$(DO_EPSCBBOX)

PLOT_DIRS:=$(call ifundefined,PLOT_DIRS,.)
PLOT_DIRS+=$(SUB_DIRS)

PLOT_GPI:=$(call listfiles,*.$(GNUPLOT_EXT),$(PLOT_DIRS))

gpiterm = $(shell $(SED) -e 's/\#.*//' $(1) | $(GREP) "set term" | $(SED) -e 'h;s/set term\(inal\)\{0,1\} \([a-z]*\)/\2/;:l1;s/\([a-z_]*\)\(.*\) \(eps\|pdf\)\( .*\|\)$$/\1_\3\2\4/;t l1;:l2;s/\([a-z_]*\)\(.*\) \(standalone\|input\)\( .*\|\)$$/\1_\3\2\4/;t l2;s/\([a-z_]*\).*/\1/')

MAKE_PLOT_TERMINALS:=
MAKE_PLOT_TERMINALS+=png
MAKE_PLOT_TERMINALS+=svg
MAKE_PLOT_TERMINALS+=postscript postscript_eps
MAKE_PLOT_TERMINALS+=pdf
MAKE_PLOT_TERMINALS+=epscairo pdfcairo pngcairo
MAKE_PLOT_TERMINALS+=cairolatex cairolatex_input cairolatex_standalone
MAKE_PLOT_TERMINALS+=cairolatex_eps cairolatex_eps_input cairolatex_eps_standalone
MAKE_PLOT_TERMINALS+=cairolatex_pdf cairolatex_pdf_input cairolatex_pdf_standalone
MAKE_PLOT_TERMINALS+=epslatex epslatex_input epslatex_standalone
MAKE_PLOT_TERMINALS+=dumb

$(foreach t,$(MAKE_PLOT_TERMINALS),$(eval PLOT_GPI_$(t):=))
$(foreach f,$(PLOT_GPI),$(eval PLOT_GPI_$(call gpiterm,$(f))+=$(f)))

PLOT_GPI_postscript_eps+=$(PLOT_GPI_postscript)
PLOT_GPI_cairolatex_eps+=$(PLOT_GPI_cairolatex)
PLOT_GPI_cairolatex_eps_input+=$(PLOT_GPI_cairolatex_input)
PLOT_GPI_cairolatex_eps_standalone+=$(PLOT_GPI_cairolatex_standalone)
PLOT_GPI_cairolatex_eps_input+=$(PLOT_GPI_cairolatex_eps)
PLOT_GPI_cairolatex_pdf_input+=$(PLOT_GPI_cairolatex_pdf)
PLOT_GPI_epslatex_input+=$(PLOT_GPI_epslatex)

PLOT_PNG:=$(addsuffix .png,$(basename $(PLOT_GPI_png) $(PLOT_GPI_pngcairo)))
PLOT_CONVERT:=$(if $(MAKE_PLOT_CONVERT),$(addsuffix $(MAKE_PLOT_CONVERT),$(basename $(PLOT_PNG))))
PLOT_SVG:=$(addsuffix .svg,$(basename $(PLOT_GPI_svg)))
PLOT_EPS:=$(addsuffix .eps,$(basename $(PLOT_GPI_postscript_eps) $(PLOT_GPI_epscairo)))
PLOT_PDF:=$(addsuffix .pdf,$(basename $(PLOT_GPI_pdf) $(PLOT_GPI_pdfcairo)))
PLOT_TEX:=
PLOT_EPSTEX:=$(addsuffix .eps,$(basename $(PLOT_GPI_cairolatex_eps_input)) $(addsuffix -inc,$(basename $(PLOT_GPI_cairolatex_eps_standalone))))
PLOT_TEXEPS:=$(addsuffix .tex,$(basename $(PLOT_GPI_cairolatex_eps_input) $(PLOT_GPI_cairolatex_eps_standalone)))
PLOT_EPSTEX+=$(addsuffix .eps,$(basename $(PLOT_GPI_epslatex_input)) $(addsuffix -inc,$(basename $(PLOT_GPI_epslatex_standalone))))
PLOT_TEXEPS+=$(addsuffix .tex,$(basename $(PLOT_GPI_epslatex_input) $(PLOT_GPI_epslatex_standalone)))
PLOT_PDFTEX:=$(addsuffix .pdf,$(basename $(PLOT_GPI_cairolatex_pdf_input)) $(addsuffix -inc,$(basename $(PLOT_GPI_cairolatex_pdf_standalone))))
PLOT_TEXPDF+=$(addsuffix .tex,$(basename $(PLOT_GPI_cairolatex_pdf_input) $(PLOT_GPI_cairolatex_pdf_standalone)))
PLOT_TXT:=$(addsuffix .txt,$(basename $(PLOT_GPI_dumb)))

PLOT_PDF_EPS:=$(addsuffix .pdf,$(basename $(PLOT_EPS)))
PLOT_PDF_EPSTEX:=$(addsuffix .pdf,$(basename $(PLOT_EPSTEX)))
PLOT_EPSTOPDF:=$(PLOT_PDF_EPS) $(PLOT_PDF_EPSTEX) $(PLOT_TEXEPS)

ifneq ($(MAKE_PLOT_LATEX_PREVIEW),)
 x:=$(basename $(PLOT_GPI_epslatex_input) $(PLOT_GPI_cairolatex_eps_input) $(PLOT_GPI_cairolatex_pdf_input))
 PLOT_TEX_PDFLATEX_PREVIEW:=$(addsuffix -preview.tex,$(x))
 PLOT_PDF_PDFLATEX_PREVIEW:=$(addsuffix -preview.pdf,$(x))
endif
ifneq ($(MAKE_PLOT_LATEX_STANDALONE),)
 x:=$(basename $(PLOT_GPI_epslatex_standalone) $(PLOT_GPI_cairolatex_eps_standalone) $(PLOT_GPI_cairolatex_pdf_standalone))
 PLOT_TEX_PDFLATEX_STANDALONE:=$(addsuffix -standalone.tex,$(x))
 PLOT_PDF_PDFLATEX_STANDALONE:=$(addsuffix -standalone.pdf,$(x))
endif
PLOT_TEX_PDFLATEX:=$(PLOT_TEX_PDFLATEX_PREVIEW) $(PLOT_TEX_PDFLATEX_STANDALONE)
PLOT_PDF_PDFLATEX:=$(PLOT_PDF_PDFLATEX_PREVIEW) $(PLOT_PDF_PDFLATEX_STANDALONE)

PLOT_EPS_:=$(PLOT_EPS) $(PLOT_EPSTEX) $(PLOT_EPS_EXTRA)
PLOT_PDF_:=$(PLOT_PDF) $(PLOT_PDFTEX) $(PLOT_PDF_EPS) $(PLOT_PDF_EPSTEX) $(PLOT_PDF_PDFLATEX) $(PLOT_PDF_EXTRA)
PLOT_TEX_:=$(PLOT_TEX) $(PLOT_TEXEPS) $(PLOT_TEXPDF)

PLOT_CLEAR+=$(foreach f,$(PLOT_PDF_PDFLATEX),$(call maketemps,$(f),.tex $(PDFLATEX_TMP_EXTS)))


# image config
MAKE_IMAGE_LATEX_ICONV?=$(MAKE_LATEX_ICONV)
MAKE_IMAGE_LATEX_PREVIEW?=$(MAKE_LATEX_PREVIEW)
MAKE_IMAGE_LATEX_PREVIEW_INPUTENC?=$(MAKE_LATEX_PREVIEW_INPUTENC)
MAKE_IMAGE_LATEX_PREVIEW_CLASS?=$(MAKE_LATEX_PREVIEW_CLASS)
MAKE_IMAGE_LATEX_PREVIEW_CLASS_OPTIONS?=$(MAKE_LATEX_PREVIEW_CLASS_OPTIONS)
MAKE_IMAGE_LATEX_PREVIEW_INPUT?=$(MAKE_LATEX_PREVIEW_INPUT)
MAKE_IMAGE_LATEX_PREVIEW_SIZE?=$(MAKE_LATEX_PREVIEW_SIZE)
MAKE_IMAGE_LATEX_PREVIEW_RULE?=$(MAKE_LATEX_PREVIEW_RULE)
MAKE_IMAGE_LATEX_PREVIEW_FBOX?=$(MAKE_LATEX_PREVIEW_FBOX)

MAKE_IMAGE_CONVERT?=$(MAKE_CONVERT)
MAKE_IMAGE_CONVERT_FLAGS?=$(MAKE_CONVERT_FLAGS)

DO_IMAGE_PS2EPSI?=$(DO_PS2EPSI)
DO_IMAGE_GSBBOX?=$(DO_GSBBOX)
DO_IMAGE_EPSCBBOX?=$(DO_EPSCBBOX)

IMAGE_DIRS:=$(call ifundefined,IMAGE_DIRS,.)
IMAGE_DIRS+=$(SUB_DIRS)

IMAGE_SVG:=$(call listfiles,*.svg,$(IMAGE_DIRS))

IMAGE_EPSPDF_EXT:=$(if $(MAKE_IMAGE_EPSTOPDF),EPS eps,PDF pdf)
IMAGE_EPSPDF_EXT_UC:=$(word 1,$(IMAGE_EPSPDF_EXT))
IMAGE_EPSPDF_EXT_LC:=$(word 2,$(IMAGE_EPSPDF_EXT))

IMAGE_PNG:=$(addsuffix .png,$(basename $(IMAGE_SVG)))
IMAGE_CONVERT:=$(if $(MAKE_IMAGE_CONVERT),$(addsuffix $(MAKE_IMAGE_CONVERT),$(basename $(IMAGE_PNG))))
IMAGE_EPS:=
IMAGE_PDF:=
IMAGE_TEX:=
IMAGE_EPSTEX:=
IMAGE_TEXEPS:=
IMAGE_PDFTEX:=
IMAGE_TEXPDF:=
ifneq ($(MAKE_IMAGE_LATEX),)
 IMAGE_$(IMAGE_EPSPDF_EXT_UC)TEX+=$(addsuffix .$(IMAGE_EPSPDF_EXT_LC),$(basename $(IMAGE_SVG)))
 IMAGE_TEX$(IMAGE_EPSPDF_EXT_UC)+=$(addsuffix .tex,$(basename $(IMAGE_SVG)))
else
 IMAGE_$(IMAGE_EPSPDF_EXT_UC)+=$(addsuffix .$(IMAGE_EPSPDF_EXT_LC),$(basename $(IMAGE_SVG)))
endif

IMAGE_PDF_EPS:=$(addsuffix .pdf,$(basename $(IMAGE_EPS)))
IMAGE_PDF_EPSTEX:=$(addsuffix .pdf,$(basename $(IMAGE_EPSTEX)))
IMAGE_EPSTOPDF:=$(IMAGE_PDF_EPS) $(IMAGE_PDF_EPSTEX) $(IMAGE_TEXEPS)

ifneq ($(MAKE_IMAGE_LATEX_PREVIEW),)
 x:=$(basename $(IMAGE_EPSTEX) $(IMAGE_PDFTEX))
 IMAGE_TEX_PDFLATEX_PREVIEW:=$(addsuffix -preview.tex,$(x))
 IMAGE_PDF_PDFLATEX_PREVIEW:=$(addsuffix -preview.pdf,$(x))
endif
IMAGE_TEX_PDFLATEX:=$(IMAGE_TEX_PDFLATEX_PREVIEW)
IMAGE_PDF_PDFLATEX:=$(IMAGE_PDF_PDFLATEX_PREVIEW)

IMAGE_EPS_:=$(IMAGE_EPS) $(IMAGE_EPSTEX) $(IMAGE_EPS_EXTRA)
IMAGE_PDF_:=$(IMAGE_PDF) $(IMAGE_PDFTEX) $(IMAGE_PDF_PDFLATEX) $(IMAGE_PDF_EXTRA)
IMAGE_TEX_:=$(IMAGE_TEX) $(IMAGE_TEXEPS) $(IMAGE_TEXPDF)
IMAGE_PDF_+=$(addsuffix .pdf,$(basename $(IMAGE_EPS_)))

IMAGE_CLEAR+=$(foreach f,$(IMAGE_PDF_PDFLATEX),$(call maketemps,$(f),.tex $(PDFLATEX_TMP_EXTS)))


# tex config
TEX_MAIN_TEX:=$(wildcard $(addsuffix .tex,$(TEX_MAIN)))
TEX_MAIN:=$(basename $(TEX_MAIN_TEX))

TEX_MAIN_FLAT:=$(addsuffix -flat.tex,$(TEX_MAIN))
TEX_MAIN_DVI:=$(addsuffix .dvi,$(TEX_MAIN))
TEX_MAIN_PS:=$(addsuffix .ps,$(TEX_MAIN))
TEX_MAIN_PDF:=$(addsuffix .pdf,$(TEX_MAIN))
TEX_MAIN_PDF_2:=$(addsuffix _2.pdf,$(TEX_MAIN))
TEX_MAIN_PDF_4:=$(addsuffix _4.pdf,$(TEX_MAIN))
TEX_MAIN_PRINT_PDF:=$(addsuffix _print.pdf,$(TEX_MAIN))
TEX_MAIN_PRINT_PDF_2:=$(addsuffix _print_2.pdf,$(TEX_MAIN))
TEX_MAIN_PRINT_PDF_4:=$(addsuffix _print_4.pdf,$(TEX_MAIN))

TEX_MAIN_TMP:=$(foreach main,$(TEX_MAIN),$(addprefix $(main),$(TEX_MAIN_TMP_EXTS) $(TEX_BIB_TMP_EXTS) $(TEX_IDX_TMP_EXTS) $(TEX_NOM_TMP_EXTS) $(TEX_GLS_TMP_EXTS)))

TEX_SUB_TEX+=$(filter-out $(TEX_MAIN_TEX) $(TEX_GRAPHICSPATH),$(wildcard *.tex)) $(TEX_GRAPHICSPATH)

TEX_SUB_TEX+=$(wildcard $(addsuffix /*.tex,$(SUB_DIRS)))

TEX_SUB_TEX:=$(filter-out $(foreach main,$(TEX_MAIN),$($(main)_TEX_SUB)), $(TEX_SUB_TEX))

TEX_REF_BIB+=$(filter-out $(TEX_REF_BIB), $(wildcard *.bib))

TEX_MBIB_AUX:=$(addsuffix .aux,$(TEX_MBIB))
TEX_MBIB_TMP:=$(foreach mbib,$(TEX_MBIB),$(addprefix $(mbib),$(TEX_BIB_TMP_EXTS)))

TEX_MIDX_IDX:=$(addsuffix .idx,$(TEX_MIDX))
TEX_MIDX_TMP:=$(foreach midx,$(TEX_MIDX),$(addprefix $(midx),$(TEX_IDX_TMP_EXTS)))

TEX_CLEAR+=$(TEX_MAIN_TMP) $(TEX_MBIB_TMP) $(TEX_MIDX_TMP)

TEX_RERUN:='(There were undefined (references|citations)|Rerun to get (cross-references|the bars|citations|outlines) (right|correct))'
TEX_RERUNBIB:='No file.*\.(bbl|toc)|Citation.*undefined'
TEX_RERUNIDX:='No file.*\.ind'
TEX_RERUNNOM:='Writing nomenclature file .*\.nlo'
TEX_RERUNGLS:='\.gls'
TEX_UNDEF:='There were undefined (references|citations)'
TEX_UNDEFINED:='((Reference|Citation).*undefined)|(Label.*multiply defined)'

TEX_MAX_RERUN?=3
TEX_EXTRA_RERUN?=0


# misc.
DEPS+=Makefile Makefile.in
XDEPS:=$(wildcard $(DEPS))
TEX_XDEPS:=$(wildcard $(TEX_DEPS)) $(TEX_PREREQS)
GPI_XDEPS:=$(wildcard $(GPI_DEPS))
SVG_XDEPS:=$(wildcard $(SVG_DEPS))

# commands
define do_image_iconv
$(ICONV) -f utf-8 -t $(MAKE_IMAGE_LATEX_ICONV)
endef

do_ps2epsi = $(PS2EPSI) "$(1)" "$(1)i" && mv "$(1)i" "$(1)"

do_gsbbox = $(SED) -i -e "s/%%BoundingBox:.*/`$(GS) -dNOPAUSE -sDEVICE=bbox $(1) quit.ps 2>&1 | $(GREP) "%%BoundingBox"`/" "$(1)"

do_epscbbox = $(EPSCBBOX) "$(2)" "$(1)"

do_epstopdf = $(EPSTOPDF) "$(1)"

do_cmd = (echo "$(1)" ; $(1))


# all
_all: $(ALL)


# plot file rules
$(PLOT_PNG): V_TYPE:=png
$(PLOT_SVG): V_TYPE:=svg
$(PLOT_EPS): V_TYPE:=eps
$(PLOT_PDF): V_TYPE:=pdf
$(PLOT_EPSTEX) $(PLOT_TEXEPS): V_TYPE:=epstex
$(PLOT_PDFTEX) $(PLOT_TEXPDF): V_TYPE:=pdftex
$(PLOT_TXT): V_TYPE:=txt
$(PLOT_PDF_EPS): V_TYPE:=eps
$(PLOT_PDF_EPSTEX): V_TYPE:=epstex
$(PLOT_EPSTOPDF): V_EPSTOPDF:=yes

%.png %.svg %-inc.eps %.eps %-inc.pdf %.pdf %.tex %.txt: %.$(GNUPLOT_EXT) $(GPI_DAT) $(GPI_XDEPS) $(XDEPS)
#	@echo "V_TYPE: $(V_TYPE), IN: $<, OUT: $@"
	@$(call do_cmd,$(call insubdir,$(GNUPLOT),$<))
	@if [ "$(V_TYPE)" = "epstex" -o "$(V_TYPE)" = "pdftex" ] ; then \
	  if [ "$(V_TYPE)" = "epstex" ] ; then e="eps" ; else e="pdf" ; fi ; \
	  b="$(basename $<)" ; \
	  f="$(basename $@).$${e}" ; \
	  n="$(notdir $(basename $@))" ; \
	  $(call do_cmd,$(MV) "$${b}.tex" "$${f}_tex") ; \
	  $(call do_cmd,$(CAT) "$${f}_tex" $(if $(MAKE_IMAGE_LATEX_ICONV),| $(do_image_iconv)) > "$${b}.tex" ; $(RM) $(RM_FLAGS) "$${f}_tex") ; \
	fi
	@if [ "$(V_TYPE)" = "eps" -o "$(V_TYPE)" = "epstex" ] ; then \
	  f="$(basename $@).eps" ; \
	  if [ -n "$(DO_PLOT_PS2EPSI)" -a -f "$${f}" ] ; then $(call do_cmd,$(call do_ps2epsi,$${f})) ; fi ; \
	  if [ -n "$(DO_PLOT_GSBBOX)" -a -f "$${f}" ] ; then $(call do_cmd,$(call do_gsbbox,$${f})) ; fi ; \
	  if [ -n "$(DO_PLOT_EPSCBBOX)" -a -f "$${f}" ] ; then $(call do_cmd,$(call do_epscbbox,$${f},$(DO_PLOT_EPSCBBOX))) ; fi ; \
	fi
#	@echo "V_EPSTOPDF: '$(V_EPSTOPDF)'"
	@if [ -n "$(V_EPSTOPDF)" ] ; then \
	  f="$(basename $@).eps" ; \
	  if [ -f "$${f}" ] ; then $(call do_cmd,$(call do_epstopdf,$${f})) ; fi ; \
	fi

%-standalone.tex: %.tex $(XDEPS)
	$(CP) $< $@

%-standalone.pdf: %-standalone.tex %-inc.pdf $(XDEPS)
	$(call insubdir,$(PDFLATEX) $(PDFLATEX_FLAGS),$<)
	$(RM) $(RM_FLAGS) $(addprefix $(basename $<),.tex $(PDFLATEX_TMP_EXTS))

%$(MAKE_PLOT_CONVERT): %.png $(XDEPS)
	@$(call do_cmd,$(CONVERT) $(CONVERT_FLAGS) $(MAKE_PLOT_CONVERT_FLAGS) $< $@)


# image file rules
$(IMAGE_PNG): V_TYPE:=png
$(IMAGE_EPS): V_TYPE:=eps
$(IMAGE_PDF): V_TYPE:=pdf
$(IMAGE_EPSTEX) $(IMAGE_TEXEPS): V_TYPE:=epstex
$(IMAGE_PDFTEX) $(IMAGE_TEXPDF): V_TYPE:=pdftex
$(IMAGE_PDF_EPS): V_TYPE:=eps
$(IMAGE_PDF_EPSTEX): V_TYPE:=epstex
$(IMAGE_EPSTOPDF): V_EPSTOPDF:=yes

INKSCAPE_FLAGS+=
INKSCAPE_FLAGS_png+=-e
INKSCAPE_FLAGS_eps+=-E
INKSCAPE_FLAGS_pdf+=-A
INKSCAPE_FLAGS_epstex+=--export-latex -P
INKSCAPE_FLAGS_pdftex+=--export-latex -A

%.png %.eps %.pdf %.tex: %.svg $(SVG_XDEPS) $(XDEPS)
#	@echo "V_TYPE: $(V_TYPE), IN: $<, OUT: $@"
	@$(call do_cmd,$(INKSCAPE) $(INKSCAPE_FLAGS) $(INKSCAPE_FLAGS_$(V_TYPE)) $(basename $@).$(subst tex,,$(V_TYPE)) $<)
	@if [ "$(V_TYPE)" = "epstex" -o "$(V_TYPE)" = "pdftex" ] ; then \
	  e="$(subst tex,,$(V_TYPE))" ; \
	  b="$(basename $<)" ; \
	  f="$(basename $@).$${e}" ; \
	  n="$(notdir $(basename $@))" ; \
	  $(call do_cmd,$(SED) -e "s/{$${n}\.$${e}}/{$${n}}/" "$${f}_tex" $(if $(MAKE_IMAGE_LATEX_ICONV),| $(do_image_iconv)) > "$${b}.tex" ; $(RM) $(RM_FLAGS) "$${f}_tex") ; \
	fi
	@if [ "$(V_TYPE)" = "eps" -o "$(V_TYPE)" = "epstex" ] ; then \
	  f="$(basename $@).eps" ; \
	  if [ -n "$(DO_IMAGE_PS2EPSI)" -a -f "$${f}" ] ; then $(call do_cmd,$(call do_ps2epsi,$${f})) ; fi ; \
	  if [ -n "$(DO_IMAGE_GSBBOX)" -a -f "$${f}" ] ; then $(call do_cmd,$(call do_gsbbox,$${f})) ; fi ; \
	  if [ -n "$(DO_IMAGE_EPSCBBOX)" -a -f "$${f}" ] ; then $(call do_cmd,$(call do_epscbbox,$${f},$(DO_IMAGE_EPSCBBOX))) ; fi ; \
	fi
#	@echo "V_EPSTOPDF: $(V_EPSTOPDF)"
	@if [ -n "$(V_EPSTOPDF)" ] ; then \
	  f="$(basename $@).eps" ; \
	  if [ -f "$${f}" ] ; then $(call do_cmd,$(call do_epstopdf,$${f})) ; fi ; \
	fi

%$(MAKE_IMAGE_CONVERT): %.png $(XDEPS)
	@$(call do_cmd,$(CONVERT) $(CONVERT_FLAGS) $(MAKE_IMAGE_CONVERT_FLAGS) $< $@)


# plot and image file rules
$(PLOT_TEX_PDFLATEX_PREVIEW): V_CLASS:=$(MAKE_PLOT_LATEX_PREVIEW_CLASS)
$(PLOT_TEX_PDFLATEX_PREVIEW): V_CLASS_OPTIONS:=$(MAKE_PLOT_LATEX_PREVIEW_CLASS_OPTIONS)
$(PLOT_TEX_PDFLATEX_PREVIEW): V_INPUTENC:=$(MAKE_PLOT_LATEX_PREVIEW_INPUTENC)
$(PLOT_TEX_PDFLATEX_PREVIEW): V_INPUT:=$(MAKE_PLOT_LATEX_PREVIEW_INPUT)
$(PLOT_TEX_PDFLATEX_PREVIEW): V_SIZE:=$(MAKE_PLOT_LATEX_PREVIEW_SIZE)
$(PLOT_TEX_PDFLATEX_PREVIEW): V_RULE:=$(MAKE_PLOT_LATEX_PREVIEW_RULE)
$(PLOT_TEX_PDFLATEX_PREVIEW): V_FBOX:=$(MAKE_PLOT_LATEX_PREVIEW_FBOX)

$(IMAGE_TEX_PDFLATEX_PREVIEW): V_CLASS:=$(MAKE_IMAGE_LATEX_PREVIEW_CLASS)
$(IMAGE_TEX_PDFLATEX_PREVIEW): V_CLASS_OPTIONS:=$(MAKE_IMAGE_LATEX_PREVIEW_CLASS_OPTIONS)
$(IMAGE_TEX_PDFLATEX_PREVIEW): V_INPUTENC:=$(MAKE_IMAGE_LATEX_PREVIEW_INPUTENC)
$(IMAGE_TEX_PDFLATEX_PREVIEW): V_INPUT:=$(MAKE_IMAGE_LATEX_PREVIEW_INPUT)
$(IMAGE_TEX_PDFLATEX_PREVIEW): V_SIZE:=$(MAKE_IMAGE_LATEX_PREVIEW_SIZE)
$(IMAGE_TEX_PDFLATEX_PREVIEW): V_RULE:=$(MAKE_IMAGE_LATEX_PREVIEW_RULE)
$(IMAGE_TEX_PDFLATEX_PREVIEW): V_FBOX:=$(MAKE_IMAGE_LATEX_PREVIEW_FBOX)

# *-preview.tex files need to be intermediate files to prevent their 'remaking' (setting the target-specific variables disables their automatic intermediate property)
.INTERMEDIATE: $(PLOT_TEX_PDFLATEX_PREVIEW) $(IMAGE_TEX_PDFLATEX_PREVIEW)

%-preview.tex: %.tex $(XDEPS)
	@echo "V_CLASS: $(V_CLASS), V_CLASS_OPTIONS: $(V_CLASS_OPTIONS), V_INPUTENC: $(V_INPUTENC)"
	@echo "\\\documentclass[$(V_CLASS_OPTIONS)]{$(V_CLASS)}" > $@
	@$(if $(V_INPUTENC),echo "\\\usepackage[$(V_INPUTENC)]{inputenc}" >> $@)
	@echo "\\\usepackage{graphicx}" >> $@
	@echo "\\\usepackage{xcolor}"   >> $@
	@$(if $(V_INPUT),echo "\\\input{$(V_INPUT)}" >> $@)
	@echo "\\\begin{document}" >> $@
	@echo "\\\begin{center}"   >> $@
	@$(if $(V_SIZE),echo "\\\$(V_SIZE)" >> $@)
	@$(if $(V_FBOX),echo "\\\fbox{%" >> $@)
	@$(if $(V_RULE),echo "\\\rule{$(V_RULE)}{1pt}\par" >> $@)
	@echo "\\\input{$(basename $(notdir $<))}\par" >> $@
	@$(if $(V_RULE),echo "\\\rule{$(V_RULE)}{1pt}\par" >> $@)
	@$(if $(V_FBOX),echo "}%" >> $@)
	@echo "\\\end{center}"   >> $@
	@echo "\\\end{document}" >> $@

%-preview.pdf: %-preview.tex %.pdf $(XDEPS)
	$(call insubdir,$(PDFLATEX) $(PDFLATEX_FLAGS),$<)
	$(RM) $(RM_FLAGS) $(addprefix $(basename $<),.tex $(PDFLATEX_TMP_EXTS))

%.pdf: %.eps $(XDEPS)
	$(EPSTOPDF) $<

%-flat.tex: %.tex $(XDEPS)
	$(LATEXPAND) -o $@ $<

# tex file rules
%.dvi: %.tex $(TEX_XDEPS) $(XDEPS)
	$(RM) $(addprefix $(basename $@),$(TEX_MAIN_TMP_EXTS) $(TEX_BIB_TMP_EXTS) $(TEX_IDX_TMP_EXTS)) $(TEX_MBIB_TMP) $(TEX_MIDX_TMP)
	@echo "**** LaTeX first pass" ; echo "$(LATEX) $<" ; $(LATEX) $<
ifeq ($(DRAFT),)
	@if $(EGREP) -q -e $(TEX_RERUNBIB) -e $(TEX_RERUNIDX) -e $(TEX_RERUNNOM) $(basename $@).log ; then \
	  if $(EGREP) -q $(TEX_RERUNBIB) $(basename $@).log ; then \
	    echo "**** BibTeX" ; $(foreach aux,$(basename $@).aux $(TEX_MBIB_AUX),if [ -f "$(aux)" ] ; then echo "$(BIBTEX) $(aux)" ; $(BIBTEX) $(aux) | grep -v "Warning--empty language" ; fi ; ) \
	  fi ; \
	  if $(EGREP) -q $(TEX_RERUNIDX) $(basename $@).log ; then \
	    echo "**** makeindex (idx)" ; $(foreach idx,$(basename $@).idx $(TEX_MIDX_IDX),if [ -f "$(idx)" ] ; then echo "$(MAKEIDX) $(idx)" ; $(MAKEIDX) $(idx) ; fi ; ) \
	  fi ; \
	  if $(EGREP) -q $(TEX_RERUNNOM) $(basename $@).log ; then \
	    echo "**** makeindex (nomencl)" ; if [ -f "$(basename $@).nlo" ] ; then echo "$(MAKEIDX) $(basename $@).nlo -s nomencl.ist -o $(basename $@).nls" ; $(MAKEIDX) $(basename $@).nlo -s nomencl.ist -o $(basename $@).nls ; fi ; \
	  fi ; \
	  echo "**** LaTeX BibTeX and/or makeindex pass" ; echo "$(LATEX) $<" ; $(LATEX) $< ; \
	fi
	@i=1 ; while [ $${i} -le $(TEX_MAX_RERUN) ] ; do \
	  if $(EGREP) -q $(TEX_RERUN) $(basename $@).log ; then \
	    echo "**** LaTeX rerun ($${i})" ; echo "$(LATEX) $<" ; $(LATEX) $< ; \
	  else \
	    break ; \
	  fi ; \
	  i=$$((i+1)) ; \
	done
	@i=1 ; while [ $${i} -le $(TEX_EXTRA_RERUN) ] ; do \
	  echo "**** LaTeX extra rerun ($${i})" ; echo "$(LATEX) $<" ; $(LATEX) $< ; \
	  i=$$((i+1)) ; \
	done
	@if $(EGREP) -q $(TEX_UNDEF) $(basename $@).log ; then \
	  echo "**** Undefined references and citations:" ; $(EGREP) -i $(TEX_UNDEFINED) $(basename $@).log || echo "None." ; \
	fi
endif

%.ps: %.dvi $(XDEPS)
	$(DVIPS) -f $< > $@

ifneq ($(USE_XELATEX),)
 PDFLATEX_CMD:=$(XELATEX)
 PDFLATEX_NAME:=XeLaTeX
else
 PDFLATEX_CMD:=$(PDFLATEX) $(PDFLATEX_FLAGS)
 PDFLATEX_NAME:=LaTeX
endif

%.pdf: %.tex $(TEX_XDEPS) $(XDEPS)
	$(RM) $(RM_FLAGS) $(addprefix $(basename $@),$(TEX_MAIN_TMP_EXTS) $(TEX_BIB_TMP_EXTS) $(TEX_IDX_TMP_EXTS)) $(TEX_MBIB_TMP) $(TEX_MIDX_TMP)
	@$(foreach sub,$(FIG_SUB_SOURCE),if [ -f "${sub}/Makefile" ] ; then echo "**** Figures in $(sub)" ; echo "$(MAKE) FIG_SUB_DIR=$(sub) -C $(sub) fig_pdf" ; $(MAKE) FIG_SUB_DIR=$(sub) -C $(sub) fig_pdf ; fi ;)
	@echo "**** PDF $(PDFLATEX_NAME) first pass" ; echo "$(PDFLATEX_CMD) $<" ; $(PDFLATEX_CMD) $<
ifeq ($(DRAFT),)
	@if $(EGREP) -q -e $(TEX_RERUNBIB) -e $(TEX_RERUNIDX) -e $(TEX_RERUNNOM) $(basename $@).log ; then \
	  if $(EGREP) -q $(TEX_RERUNBIB) $(basename $@).log ; then \
	    echo "**** BibTeX" ; $(foreach aux,$(basename $@).aux $(TEX_MBIB_AUX),if [ -f "$(aux)" ] ; then echo "$(BIBTEX) $(aux)" ; $(BIBTEX) $(aux) | grep -v "Warning--empty language" ; fi ; ) \
	  fi ; \
	  if $(EGREP) -q $(TEX_RERUNIDX) $(basename $@).log ; then \
	    echo "**** makeindex (idx)" ; $(foreach idx,$(basename $@).idx $(TEX_MIDX_IDX),if [ -f "$(idx)" ] ; then echo "$(MAKEIDX) $(idx)" ; $(MAKEIDX) $(idx) ; fi ; ) \
	  fi ; \
	  if $(EGREP) -q $(TEX_RERUNNOM) $(basename $@).log ; then \
	    echo "**** makeindex (nomencl)" ; if [ -f "$(basename $@).nlo" ] ; then echo "$(MAKEIDX) $(basename $@).nlo -s nomencl.ist -o $(basename $@).nls" ; $(MAKEIDX) $(basename $@).nlo -s nomencl.ist -o $(basename $@).nls ; fi ; \
	  fi ; \
	  if $(EGREP) -q $(TEX_RERUNGLS) $(basename $@).log ; then \
	    echo "**** makeglossaries" ; if [ -f "$(basename $@).glo" ] ; then echo "$(MAKEGLS) $(basename $@).glo" ; $(MAKEGLS) $(basename $@).glo ; fi ; \
	  fi ; \
	  echo "**** PDF $(PDFLATEX_NAME) BibTeX, makeindex, and/or makeglossaries pass" ; echo "$(PDFLATEX_CMD) $<" ; $(PDFLATEX_CMD) $< ; \
	fi
	@i=1 ; while [ $${i} -le $(TEX_MAX_RERUN) ] ; do \
	  if $(EGREP) -q $(TEX_RERUN) $(basename $@).log ; then \
	    echo "**** PDF $(PDFLATEX_NAME) rerun ($${i})" ; echo "$(PDFLATEX_CMD) $<" ; $(PDFLATEX_CMD) $< ; \
	  else \
	    break ; \
	  fi ; \
	  i=$$((i+1)) ; \
	done
	@if [ -f "$(basename $@).glo" -a -n "$(TEX_EXTRA_GLS_RERUN)" ] ; then \
	  echo "**** makeglossaries extra rerun" ; echo "$(MAKEGLS) $(basename $@).glo" ; $(MAKEGLS) $(basename $@).glo ; \
	  echo "**** PDF $(PDFLATEX_NAME) extra rerun (makeglossaries)" ; echo "$(PDFLATEX_CMD) $<" ; $(PDFLATEX_CMD) $< ; \
	fi
	@i=1 ; while [ $${i} -le $(TEX_EXTRA_RERUN) ] ; do \
	  echo "**** PDF $(PDFLATEX_NAME) extra rerun ($${i})" ; echo "$(PDFLATEX_CMD) $<" ; $(PDFLATEX_CMD) $< ; \
	  i=$$((i+1)) ; \
	done
	@if $(EGREP) -q $(TEX_UNDEF) $(basename $@).log ; then \
	  echo "**** Undefined references and citations:" ; $(EGREP) -i $(TEX_UNDEFINED) $(basename $@).log || echo "None." ; \
	fi
endif

ifneq ($(PDF_PRINT),)
%_print.pdf: %.pdf $(XDEPS)
	$(PDFTK) $< cat $(PDF_PRINT) output $@
endif

%_2.pdf: %.pdf $(XDEPS)
	$(PDFJAM) --paper a4paper --nup 1x2 --no-landscape --outfile $@ $<

%_4.pdf: %.pdf $(XDEPS)
	$(PDFJAM) --paper a4paper --nup 2x2 --landscape --outfile $@ $<

plot_png: $(PLOT_PNG)
plot_convert: $(PLOT_CONVERT)
plot_svg: $(PLOT_SVG)
plot_eps: $(PLOT_EPS_)
plot_pdf: $(PLOT_PDF_)
plot_tex: $(PLOT_TEX_)
plot_txt: $(PLOT_TXT)

plot_clear:
	$(call ifnotempty,$(RM) $(RM_FLAGS),$(strip $(PLOT_CLEAR) $(PLOT_CLEAR_EXTRA)))

plot_clean: plot_clear
	$(call ifnotempty,$(RM) $(RM_FLAGS),$(strip $(PLOT_PNG) $(PLOT_CONVERT) $(PLOT_SVG) $(PLOT_EPS_) $(PLOT_PDF_) $(PLOT_TEX_) $(PLOT_TXT)))

plot_info:
	@echo "plot input:"
	@echo " plot-gpi: $(PLOT_GPI)"
	@$(foreach t,$(MAKE_PLOT_TERMINALS),echo " plot-$(t): $(PLOT_GPI_$(t))"; )
	@echo
	@echo "plot output:"
	@echo " plot-png: $(PLOT_PNG)"
	@echo " plot-convert: $(PLOT_CONVERT)"
	@echo " plot-svg: $(PLOT_SVG)"
	@echo " plot-eps: $(PLOT_EPS)"
	@echo " plot-epstex: $(PLOT_EPSTEX)"
	@echo " plot-pdf: $(PLOT_PDF)"
	@echo " plot-pdftex: $(PLOT_PDFTEX)"
	@echo " plot-tex: $(PLOT_TEX)"
	@echo " plot-texeps: $(PLOT_TEXEPS)"
	@echo " plot-texpdf: $(PLOT_TEXPDF)"
	@echo " plot-txt: $(PLOT_TXT)"
	@echo " plot-pdf-eps: $(PLOT_PDF_EPS)"
	@echo " plot-pdf-epstex: $(PLOT_PDF_EPSTEX)"
	@echo " plot-epstopdf: $(PLOT_EPSTOPDF)"
	@echo " plot-tex-pdflatex: $(PLOT_TEX_PDFLATEX)"
	@echo " plot-pdf-pdflatex: $(PLOT_PDF_PDFLATEX)"
	@echo " plot-eps-extra: $(PLOT_EPS_EXTRA)"
	@echo " plot-pdf-extra: $(PLOT_PDF_EXTRA)"
	@echo " plot-clear: $(PLOT_CLEAR)"
	@echo " plot-clear-extra: $(PLOT_CLEAR_EXTRA)"
	@echo
	@echo "plot misc:"
	@echo " gpi-deps: $(GPI_XDEPS)"
	@echo

#.PHONY: plot_clear plot_clean plot_info

# image rules
image_png: $(IMAGE_PNG)
image_convert: $(IMAGE_CONVERT)
image_eps: $(IMAGE_EPS_)
image_pdf: $(IMAGE_PDF_)
image_tex: $(IMAGE_TEX_)

image_clear:
	$(call ifnotempty,$(RM) $(RM_FLAGS),$(strip $(IMAGE_CLEAR) $(IMAGE_CLEAR_EXTRA)))

image_clean: image_clear
	$(call ifnotempty,$(RM) $(RM_FLAGS),$(strip $(IMAGE_PNG) $(IMAGE_CONVERT) $(IMAGE_EPS_) $(IMAGE_PDF_) $(IMAGE_TEX_)))

image_info:
	@echo "image input:"
	@echo " image-svg: $(IMAGE_SVG)"
	@echo
	@echo "image output:"
	@echo " image-png: $(IMAGE_PNG)"
	@echo " image-convert: $(IMAGE_CONVERT)"
	@echo " image-eps: $(IMAGE_EPS)"
	@echo " image-epstex: $(IMAGE_EPSTEX)"
	@echo " image-pdf: $(IMAGE_PDF)"
	@echo " image-pdftex: $(IMAGE_PDFTEX)"
	@echo " image-tex: $(IMAGE_TEX)"
	@echo " image-texeps: $(IMAGE_TEXEPS)"
	@echo " image-texpdf: $(IMAGE_TEXPDF)"
	@echo " image-pdf-eps: $(IMAGE_PDF_EPS)"
	@echo " image-pdf-epstex: $(IMAGE_PDF_EPSTEX)"
	@echo " image-epstopdf: $(IMAGE_EPSTOPDF)"
	@echo " image-tex-pdflatex: $(IMAGE_TEX_PDFLATEX)"
	@echo " image-pdf-pdflatex: $(IMAGE_PDF_PDFLATEX)"
	@echo " image-eps-extra: $(IMAGE_EPS_EXTRA)"
	@echo " image-pdf-extra: $(IMAGE_PDF_EXTRA)"
	@echo " image-clear: $(IMAGE_CLEAR)"
	@echo " image-clear-extra: $(IMAGE_CLEAR_EXTRA)"
	@echo
	@echo "image misc:"
	@echo " svg-deps: $(SVG_XDEPS)"
	@echo

# tex rules
#$(TEX_MAIN_FLAT): $(TEX_SUB_TEX) $(PLOT_EPSTEX) $(PLOT_PDFTEX) $(IMAGE_EPSTEX) $(IMAGE_PDFTEX) $(XDEPS)
$(TEX_MAIN_FLAT): $(TEX_SUB_TEX) $(XDEPS)

$(TEX_MAIN_DVI): $(TEX_SUB_TEX) $(TEX_REF_BIB) $(PLOT_EPS) $(PLOT_EPSTEX) $(PLOT_EPS_EXTRA) $(IMAGE_EPS) $(IMAGE_EPSTEX) $(IMAGE_EPS_EXTRA) $(XDEPS)

$(foreach main,$(TEX_MAIN),$(eval $(main).dvi: $($(main)_TEX_SUB)))

$(TEX_MAIN_PDF): $(TEX_SUB_TEX) $(TEX_REF_BIB) $(PLOT_PDF) $(PLOT_PDFTEX) $(PLOT_PDF_EXTRA) $(IMAGE_PDF) $(IMAGE_PDFTEX) $(IMAGE_PDF_EXTRA) $(XDEPS)

$(foreach main,$(TEX_MAIN),$(eval $(main).pdf: $($(main)_TEX_SUB)))

tex_flat: $(TEX_MAIN_FLAT)

tex_ps: $(TEX_MAIN_PS)

tex_pdf: $(TEX_MAIN_PDF)
tex_pdf_2: $(TEX_MAIN_PDF_2)
tex_pdf_4: $(TEX_MAIN_PDF_4)

tex_print_pdf: $(TEX_MAIN_PRINT_PDF)
tex_print_pdf_2: $(TEX_MAIN_PRINT_PDF_2)
tex_print_pdf_4: $(TEX_MAIN_PRINT_PDF_4)

tex_clear:
	$(call ifnotempty,$(RM) $(RM_FLAGS),$(strip $(TEX_CLEAR)))

tex_clean: tex_clear
	$(call ifnotempty,$(RM) $(RM_FLAGS),$(strip $(TEX_MAIN_FLAT) $(TEX_MAIN_DVI) $(TEX_MAIN_PS) $(TEX_MAIN_PDF) $(TEX_MAIN_PDF_2) $(TEX_MAIN_PDF_4) $(TEX_MAIN_PRINT_PDF) $(TEX_MAIN_PRINT_PDF_2) $(TEX_MAIN_PRINT_PDF_4)))

tex_info_%:
	@echo "$(subst tex_info_,,$@) info:"
	@echo " tex-sub-tex: $($(subst tex_info_,,$@)_TEX_SUB)"
	@echo

tex_info: $(foreach main,$(TEX_MAIN),tex_info_$(main))
	@echo "tex input:"
	@echo " tex-main-tex: $(TEX_MAIN_TEX)"
	@echo " tex-sub-tex: $(TEX_SUB_TEX)"
	@echo " tex-ref-bib: $(TEX_REF_BIB)"
	@echo
	@echo "tex output:"
	@echo " tex-main-flat: $(TEX_MAIN_FLAT)"
	@echo " tex-main-dvi: $(TEX_MAIN_DVI)"
	@echo " tex-main-ps: $(TEX_MAIN_PS)"
	@echo " tex-main-pdf: $(TEX_MAIN_PDF)"
	@echo " tex-main-tmp: $(TEX_MAIN_TMP)"
	@echo " tex-mbib-aux: $(TEX_MBIB_AUX)"
	@echo " tex-mbib-tmp: $(TEX_MBIB_TMP)"
	@echo " tex-midx-aux: $(TEX_MIDX_AUX)"
	@echo " tex-midx-tmp: $(TEX_MIDX_TMP)"
	@echo " tex-clear: $(TEX_CLEAR)"
	@echo
	@echo "tex misc:"
	@echo " tex-deps: $(TEX_XDEPS)"
	@echo

png: $(PNG)

convert: plot_convert image_convert

eps ps: plot_eps image_eps tex_ps

pdf: $(PDF)
pdf_2: tex_pdf_2
pdf_4: tex_pdf_4

print_pdf: tex_print_pdf
print_pdf_2: tex_print_pdf_2
print_pdf_4: tex_print_pdf_4

clear: $(CLEAR)

clean: $(CLEAN)

info: $(INFO)
	@echo "all:"
	@echo " deps: $(XDEPS)"
	@echo

tools:
	@echo "tools: $(sort $(TOOLS))"
	@for t in $(sort $(TOOLS)) ; do x=`which -- $${t}` ; if [ "$$?" = "0" ] ; then echo "  $${t}: $${x}" ; else echo "  $${t}: missing" ; fi ; done
