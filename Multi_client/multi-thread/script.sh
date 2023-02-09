for i in {1..100}
do
  gnome-terminal --tab -e "bash -c './client 8888; $SHELL'"
done