for i in {1..10}
do
  gnome-terminal --tab -e "bash -c './client 8888; $SHELL'"
done