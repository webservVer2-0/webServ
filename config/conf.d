server  {
	listen 80;
	body_size 10000000;
	max_connect 100;
	#TODO:max header 설정 추가하기 
	#max_header 4000;
	root storage/static/;
	default_file index.html;
	upload_path storage/temp/var1;
	access_log storage/log/access.log;
	error_log storage/log/error.log;
	include storage/static/mime.types;

	# 서버명과 같은 것들은 모두 영어 + 숫자 + '_' 만 지원한다. 
	server_name RyujeansToday;
	timeout 0;
	auto_index off;

	# 메서드는 구현하기로 한 GET, POST, DELETE 왜에는 에러처리 
	method GET POST DELETE;
	# 앞에 숫자, 뒤에 경로 파일 있어야 함 (반복형)
	# 에러는 반드시 해당 server config 에서 받아오는 것으로 생각한다. 
	error 400 storage/loc/400.html 403 storage/loc/403.html  404 storage/loc/404.html 501 storage/loc/501.html 505 storage/loc/505.html;

	# 필수 location(root)
	location / {
		root storage/static;
		method GET;
		default_file index.html;
		upload_path storage/temp/var1;
		auto_index off;
		# 주석 (무시 처리)
	}

	location /introduce {
		root storage/static;
		method GET;
		default_file introduce.html;
		auto_index off;
	}

	location /static {
		root storage/static;
		method GET;
		default_file static_page.html;
		auto_index off;
	}

	# 추가 기능들 있는 페이지
	location /redir {
		root storage/static;
		default_file redirection.html;
		method GET;
		auto_index off; 
		redirection http://www.naver.com; # redirection이 존재 시 다른 config 무시 비트 켜짐 
	}

	# POST 기능용 
	location /post {
		root storage/static;
		default_file test_post.html;
		# post 메서드로 들어오면 해당 entity는 root를 접근하는 것이 아니라, upload path를 활용한다. 
		method GET POST;
		auto_index off;
	}

	# DELETE 기능용 
	location /delete {
		root storage/static;
		default_file test_delete.html;
		method GET DELETE;
		auto_index off;
	}

	# AutoIndex 기능용 
	location /auto_index {
		root storage/static;
		default_file test_auto_index.html;
		method GET;
		auto_index on;
	}

	# cgi 용 
	location /cgi_1 {
		root storage/cgi;
		default_file test_cgi_1.html;
		method GET POST;
		cgi cgi.py; # 해당 내용이 켜져 있을 경우 default_file 은 없어도 됨. 
		auto_index off;
	}

	location /cgi_2 {
		root storage/cgi2;
		default_file test_cgi_2.html;
		method GET POST;
		cgi cgi.py; # 해당 내용이 켜져 있을 경우 default_file 은 없어도 됨. 
		auto_index off;
	}

}

server  {
	listen 4242;
	body_size 10000000;
	max_connect 100;
	root storage/static2/;
	default_file index.html;
	upload_path storage/temp/var2;
	access_log storage/log/access.log;
	error_log storage/log/error.log;
	include storage/static2/mime.types;

	# 서버명과 같은 것들은 모두 영어 + 숫자 + '_' 만 지원한다. 
	server_name SiegeTesting;
	timeout 1;
	auto_index off;

	error 400 storage/loc/400.html 403 storage/loc/403.html  404 storage/loc/404.html 501 storage/loc/501.html 505 storage/loc/505.html;

	# 필수 location(root)
	location / {
		root storage/static2;
		method GET;
		default_file index.html;
	}
}
