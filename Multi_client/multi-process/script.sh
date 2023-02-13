for i in {1..1000}
do
  gnome-terminal --tab -e "bash -c './client 8888; $SHELL'"
done