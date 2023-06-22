if [ ! -f "vau-test.pdf" ]
then
    echo "ERROR: output vau-test.pdf not found !!!"
    exit 1
fi
sz=$(ls -la vau-test.pdf | cut -d ' ' -f 8)
echo "size: $sz"

if [ $sz -lt 5000 ]
then
    echo "ERROR: generate blank pdf !!!"
    exit 2
fi

echo "SUCCESS"
