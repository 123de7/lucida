# Rather than forking a subshell, execute all commands
# in java-config.sh in the current shell.
source ../../config.inc

# Build OpenEphyra
echo -e "./compile-openephyra.sh: `pwd`"
echo -e "./compile-openephyra.sh: Building OpenEphyra..."
cd question-answer
ant
cd ..

# Build Wikipedia database
if [ ! -d "question-answer/wiki_indri_index" ]; then
	echo "Building Wikipedia database..."
	wget http://web.eecs.umich.edu/~jahausw/download/wiki_indri_index.tar.gz
	tar xzvf wiki_indri_index.tar.gz -C question-answer/
else
	echo "Wikipedia database has already been built"
fi

echo "OpenEphyra done"
