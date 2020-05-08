
get_passwd(){
	if [[ -z "${CYCLONE_PASS}" ]]; then
  	 passwd="default_pass"
	else
  	 passwd="${CYCLONE_PASS}"
	fi
	echo "$passwd";
}


hello_world(){
	echo "hello world"
}
