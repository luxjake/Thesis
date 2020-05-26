This folder contain a few python scripts made to help the use of RDFev.
???_gen_comp.py :
	Used to generate a file containing a list of comparison to do.
	Get as input a file listing folders (each folder containing the files of a version of the same dataset e.g a folder for each YAGO version)
???_dill_all.py :
	Run RDFev with the comparisons listed in the input file. The input file is produced with the ???_gen_comp.py scripts.
	Also summarize the comparison result in a specified output file.