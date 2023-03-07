server  {
	listen 80;
	body_size 10240;
	max_connect 200;
	root storage/static/;
	default_file index.html;
	upload_path storage/temp/var1;
	access_log storage/log/access.log;
	error_log storage/log/error.log;

	# 서버명과 같은 것들은 모두 영어 + 숫자 + '_' 만 지원한다. 
	server_name serv1;
	timeout 10;
	auto_index on;

	# 메서드는 구현하기로 한 GET, POST, DELETE 왜에는 
	method GET POST DELETE;
	# 앞에 숫자, 뒤에 경로 파일 있어야 함 (반복형)
	error 404 storage/loc/404.html 500 storage/loc/index.html;

	# 필수 location(root)
	location / {
		root storage/static;
		method GET POST DELETE;
		default_file index.html;
		upload_path storage/temp/var1;
		auto_index off;
		# 주석 (무시 처리)
		redirection storage/static;
		error 500 storage/loc/index.html; # error 는 지정되어 있지 않으면 serveer로 찾아간다. 
	}

	# 추가 기능들 있는 페이지
	location /loc {
		#root storage/loc;
		#method GET;
		default_file index.html;
		auto_index off; 
		redirection storage/static; # redirection이 존재 시 다른 config 무시 비트 켜짐 
	}

	# POST 기능용 
	location /temp {
		root storage/temp/var1;
		method GET POST;
		default_file index.html;
		auto_index off;
		#redirection storage/static2; # redirection이 존재 시 다른 config 
	}

	# cgi 용 
	location .py {
		root storage/cgi;
		default_file index.html;
		method GET POST;
		cgi cgi.py; # 해당 내용이 켜져 있을 경우 default_file 은 없어도 됨. 
		method GET;
	}

}

server  {
	listen 4242;
	body_size 10240;
	max_connect 200;
	root storage/static/;
	default_file index.html;
	upload_path storage/temp/var1;
	access_log storage/log/access.log;
	error_log storage/log/error.log;

	# 서버명과 같은 것들은 모두 영어 + 숫자 + '_' 만 지원한다. 
	server_name second_server;
	timeout 10;
	auto_index on;

	# 메서드는 구현하기로 한 GET, POST, DELETE 왜에는 
	method GET POST DELETE;
	# 앞에 숫자, 뒤에 경로 파일 있어야 함 (반복형)
	error 404 storage/loc/index.html 500 storage/loc/index.html;

	# 필수 location(root)
	location / {
		root storage/static;
		method GET POST DELETE;
		default_file index.html;
		upload_path storage/temp/var1;
		auto_index off;
		# 주석 (무시 처리)
		redirection storage/static;
		error 500 storage/loc/index.html; # error 는 지정되어 있지 않으면 serveer로 찾아간다. 
	}

	# 추가 기능들 있는 페이지
	location /loc {
		#root storage/loc;
		#method GET;
		default_file index.html;
		auto_index off; 
		redirection storage/static; # redirection이 존재 시 다른 config 무시 비트 켜짐 
	}

	# POST 기능용 
	location /temp {
		root storage/temp/var1;
		method GET POST;
		default_file index.html;
		auto_index off;
		#redirection storage/static2; # redirection이 존재 시 다른 config 
	}

	# cgi 용 
	location .py {
		root storage/cgi;
		default_file index.html;
		method GET POST;
		cgi cgi.py; # 해당 내용이 켜져 있을 경우 default_file 은 없어도 됨. 
		method GET;
	}

}
