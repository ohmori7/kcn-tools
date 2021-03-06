top_builddir=..
include $(top_builddir)/Makefile.config

SUBDIRS = man

# A little trick to simplify some of the rules.
VPATH = $(builddir):$(srcdir):$(top_srcdir)/contrib/perl

docdir = $(datadir)/doc

# Used by install-doc
HTML_DIR = $(DESTDIR)$(docdir)/$(PACKAGE)/html
PDF_DIR  = $(DESTDIR)$(docdir)/$(PACKAGE)/pdf

ASCIIDOC_CONF  = $(srcdir)asciidoc.conf
ASCIIDOC_FLAGS += -f $(ASCIIDOC_CONF) \
		  -a "builddir=$(CURDIR)/" \
		  -a asciidoc7compatible \
		  -a elinks_version=$(VERSION)

#############################################################################
# Build files

TXT_DOCS = \
 features.txt \
 keymap-actions.txt \
 keymap-defaults.txt \
 option-command.txt \
 option-config.txt

HTML_DOCS-$(CONFIG_ASCIIDOC) += \
 elinks.1.html \
 elinks.conf.5.html \
 elinkskeys.5.html \
 hacking.html \
 manual.html

HTML_DOCS-$(CONFIG_XMLTO) += \
 manual.html-chunked

HTML_DOCS-$(CONFIG_POD2HTML) += \
 perl.html \
 perl-hooks.html

MAN_DOCS-$(CONFIG_XMLTO) += \
 elinks.1 \
 elinks.conf.5 \
 elinkskeys.5

# Use jw for generating PDF, since xmlto seems to freak out.
PDF_DOCS-$(CONFIG_JW) += \
 manual.pdf


#############################################################################
## Set the default doc rules

MAN_DOCS  = $(MAN_DOCS-yes)
HTML_DOCS = $(HTML_DOCS-yes)
PDF_DOCS  = $(PDF_DOCS-yes)

txt:  $(TXT_DOCS)
html: txt $(HTML_DOCS)
pdf:  txt $(PDF_DOCS)
man:  txt $(MAN_DOCS)

all-docs: man html pdf

install-doc: all-docs update-man install
	@$(foreach doc,$(HTML_DOCS), \
		if test -d $(doc); then \
			$(MKINSTALLDIRS) $(DESTDIR)$(docdir)/$(PACKAGE)/html/$(doc); \
			$(call ncmd,installdata,$(doc)/*,$(HTML_DIR)/$(doc)); \
		else \
			$(MKINSTALLDIRS) $(DESTDIR)$(docdir)/$(PACKAGE)/html; \
			$(call ncmd,installdata,$(doc),$(HTML_DIR)); \
		fi;)
	@$(foreach doc,$(PDF_DOCS), \
		$(MKINSTALLDIRS) $(DESTDIR)$(docdir)/$(PACKAGE)/pdf; \
		$(call ncmd,installdata,$(doc),$(PDF_DIR);))

update-man: man
	@$(if $(MAN_DOCS), \
		$(call ncmd,installdata,elinks.1,$(srcdir)man/man1/elinks.1.in); \
		$(call ncmd,installdata,elinkskeys.5,$(srcdir)man/man5/); \
		$(call ncmd,installdata,elinks.conf.5,$(srcdir)man/man5/))

clean-local:
	@$(RM) -r $(TXT_DOCS) $(MAN_DOCS) $(HTML_DOCS) $(PDF_DOCS) *.tmp *.xml

# TODO: perl.pod should be pod2ized during make install. --pasky
install-local:


#############################################################################
# Generated asciidoc files

# Scripts and Dependencies
HELP2DOC = $(srcdir)tools/help2doc
CONF2DOC = $(srcdir)tools/conf2doc
KEYS2DOC = $(srcdir)tools/keys2doc
ELINKS	 = $(top_builddir)/src/elinks
FEATURES = $(top_srcdir)/features.conf
KBDBIND	 = $(top_srcdir)/src/config/kbdbind.c

# Locale env vars to override system one to ensure commands
# using elinks binary will generate texts in english
LOCALES = LC_ALL=C LANGUAGE=en

# FIXME: Keep generated .txt files relative to the source directory and files
# they are included in.
quiet_cmd_help2doc = ' [$(LINK_COLOR)HELP2DOC$(END_COLOR)]   $(RELPATH)$@'
      cmd_help2doc = $(LOCALES) $(HELP2DOC) $(ELINKS) $@ > $@

quiet_cmd_conf2doc = ' [$(LINK_COLOR)CONF2DOC$(END_COLOR)]   $(RELPATH)$@'
      cmd_conf2doc = $(LOCALES) $(CONF2DOC) $(FEATURES) > $@

quiet_cmd_keys2doc = ' [$(LINK_COLOR)KEYS2DOC$(END_COLOR)]   $(RELPATH)$@'
      cmd_keys2doc = $(LOCALES) $(KEYS2DOC) $(KBDBIND) $@ > $@

quiet_cmd_code2doc = ' [$(LINK_COLOR)CODE2DOC$(END_COLOR)]   $(RELPATH)$@'
      cmd_code2doc = $(LOCALES) $(CODE2DOC) $< > $@

features.txt: $(FEATURES) $(CONF2DOC)
	$(call cmd,conf2doc)

keymap-%.txt: $(KBDBIND) $(KEYS2DOC)
	$(call cmd,keys2doc)

option-%.txt: $(ELINKS) $(HELP2DOC)
	$(call cmd,help2doc)


#############################################################################
# Build commands and macros

      quiet_cmd_jw = '       [$(LINK_COLOR)JW$(END_COLOR)]   $(RELPATH)$@'
            cmd_jw = $(JW) -b $(2) $<

   quiet_cmd_xmlto = '    [$(LINK_COLOR)XMLTO$(END_COLOR)]   $(RELPATH)$@'
         cmd_xmlto = $(XMLTO) -o $(call outdir) $(3) $(2) $<

quiet_cmd_pod2html = ' [$(LINK_COLOR)POD2HTML$(END_COLOR)]   $(RELPATH)$@'
      cmd_pod2html = $(POD2HTML) --outfile=$@ < $<

quiet_cmd_asciidoc = ' [$(LINK_COLOR)ASCIIDOC$(END_COLOR)]   $(RELPATH)$@'
      cmd_asciidoc = $(ASCIIDOC) $(ASCIIDOC_FLAGS) -b $(call backend) -d $(call doctype) -o $@ $<

# Based on $@ find out asciidoc doctype or backend + xmlto output dir.
doctype = $(if $(findstring .1.,$@)$(findstring .5.,$@),manpage,book)
backend = $(if $(findstring .xml,$@),docbook,xhtml11)
outdir  = $(if $(findstring -chunked,$@),$@,.)

# Loosely track dependencies via asciidoc includes.
asciidoc_dep = sed -n 's/[{]builddir}//g;s@include::\(.*\)\[.*@$@: $< \1@p' < $< > .deps/$(@F).asciidoc

-include .deps/*.asciidoc


#############################################################################
# Build recipies

%.html: %.txt $(ASCIIDOC_CONF)
	$(call cmd,asciidoc,xhtml11)
	@-$(call asciidoc_dep)

%.xml: %.txt $(ASCIIDOC_CONF)
	$(call cmd,asciidoc,docbook)
	@-$(call asciidoc_dep)

%.1: %.1.xml
	$(call cmd,xmlto,man)

%.5: %.5.xml
	$(call cmd,xmlto,man)

%.html-chunked: %.xml
	$(call cmd,xmlto,html)

%.pdf: %.xml
	$(call cmd,jw,pdf)

%.html: %.pod
	$(call cmd,pod2html)

perl-%.html: %.pl
	$(call cmd,pod2html)


include $(top_srcdir)/Makefile.lib
