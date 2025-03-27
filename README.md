Project Description:
    We are taking falco output and ...

How to Run:
    - All finalized work is in 'Final Graph Work'

	- Open folder 'Falco Files' and edit the configuration file to change what variabled we are recording via falco
		- edit the values.yaml file to capture more or less data from the falco script...

    - Run this file within the desired environment, collecting system calls

    - Enter this file into the Main argument of graph_Generator.c in 'Final Graph Work', before compiling and running

    - This will generate dot files for the related system call graphs

Generating an output file:

    dot -Tpng <graphName>.dot -o <outputName>.png

    - This can be done automatically by running PNG_Generation.c with the .dot files as the arguments


** Each time you run falco you must reset it to fix the issues with dev/falco0:

    sudo falcoctl driver config --type kmod
    sudo falcoctl driver install
