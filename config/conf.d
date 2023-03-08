server  {
	listen 80;
	body_size 10240;
	max_connect 200;
	root storage/static/;
	default_file index.html;
	upload_path storage/temp/var1;
	access_log storage/log/access.log;
	error_log storage/log/error.log;
	include storage/static/mime.types;

	# 서버명과 같은 것들은 모두 영어 + 숫자 + '_' 만 지원한다. 
	server_name serv1;
	timeout 10;
	auto_index on;

	# 메서드는 구현하기로 한 GET, POST, DELETE 왜에는 
	method GET POST DELETE;
	# 앞에 숫자, 뒤에 경로 파일 있어야 함 (반복형)
	# 에러는 반드시 해당 server config 에서 받아오는 것으로 생각한다. 
	error 400 storage/loc/400.html 403 storage/loc/403.html  404 storage/loc/404.html 501 storage/loc/501.html 505 storage/loc/505.html;

	# 필수 location(root)
	location / {
		root storage/static;
		method GET POST DELETE;
		default_file index.html;
		upload_path storage/temp/var1;
		auto_index off;
		# 주석 (무시 처리)
		redirection storage/static;
	}

	# 추가 기능들 있는 페이지
	location /loc {
		#root storage/loc;
		#method GET;
		#default_file index.html;
		auto_index off; 
		redirection storage/static; # redirection이 존재 시 다른 config 무시 비트 켜짐 
	}

	# POST 기능용 
	location /temp {
		root storage/temp/var1;
		method GET POST;
		#default_file index.html;
		auto_index off;
		#redirection storage/static2; # redirection이 존재 시 다른 config 
	}

	# cgi 용 
	location .py {
		root storage/cgi;
		#default_file index.html;
		method GET POST;
		cgi cgi.py; # 해당 내용이 켜져 있을 경우 default_file 은 없어도 됨. 
		method GET;
	}

}
