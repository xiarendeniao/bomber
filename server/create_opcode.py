#encoding=utf-8

if __name__ == '__main__':
	import sys, re
	if len(sys.argv) == 1:
		raise Exception('protocol file not given')

	opcodes = dict()
	for line in file(sys.argv[1], 'r'):
		rt = re.findall('^(\d+)\.( *\w+)', line)
		if rt and len(rt) > 0:
			cmd,name = rt[0]
			opcodes[int(cmd)] = name.strip()

	opFile = file('opcodes.py', 'w')
	opFile.write('#encoding=utf-8\n\n')
	for cmd,name in opcodes.iteritems():
		opFile.write('%s\t= %d\n' % (name,cmd))
	opFile.write('\n')
	opFile.write('opcodes = {\n')
	firstLine = True
	for cmd,name in opcodes.iteritems():
		if not firstLine:
			opFile.write(',\n')
		if firstLine:
			firstLine = False
		opFile.write('\t%s\t: "%s"' % (name,name.strip()))
	opFile.write('\n\t}\n')
