//all: assign1part1, assign1part2, assign1part3
assign1part1: 
	gcc assign1part1.c -o assign1part1

assign1part2: assign1part2.o phypages.o pagetable.o 
	gcc assign1part2.o phypages.o pagetable.o -o assign1part2

assign1part2.o:
	gcc -c assign1part2.c

phypages.o:
	gcc -c phypages.c

pagetable.o:
	gcc -c pagetable.c

assign1part3:
	gcc assign1part3.c -o assign1part3

clean:
	rm -rf *o assign1part2 assign1part1 assign1part3
