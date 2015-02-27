VERSION=`git log --oneline|head -1|cut -d' ' -f1`
INCLUDES=`pkg-config --cflags glib-2.0`
CFLAGS+=$(INCLUDES) -Wall -g 
LIBS=`pkg-config --libs glib-2.0` \
		-lgsl -lgslcblas -lm 
SRCS=seg.c \
	input.c \
	strutils.c \
	xalloc.c \
	lm.c \
	trie.c \
	score.c \
	stack.c \
#	 segparse.c phonstats.c 
#		prob_dist.c predictability.c options.c print.c \
#		pub.c \
#		mdata.c \
#		ctxlex.c \
#		seg_random.c \
#		seg_lexicon.c \
#		seg_nv.c \
#		seg_combine.c \
#		seg_lexc.c \
#		peak.c \
#		threshold.c \
#		mvote.c \
#		pred.c \
#		stress.c \
#		measures.c \
#		ub.c \
#		mlist.c \
#		lex.c \
#		lexc.c \
#		cclib_debug.c \
#		lexicon.c \
#		packed_chart.c \
#		cyk.c \
#		cyk_packed.c \
#		

OBJECTS=$(SRCS:.c=.o) cmdline.o

#trietest: trie_test.o trie.o xalloc.o stack.o
#	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)


seg: $(OBJECTS)
	-echo $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)
	make depend

cmdline.h: seg.ggo 
	gengetopt --set-version=$(VERSION) < $^

all: $(OBJECTS)

test: $(OBJECTS) cgparse/lexicon.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	-rm -f *.o seg

depend:
	$(CC) $(CFLAGS) -MM -MG $(SRCS) >.depend

-include .depend
