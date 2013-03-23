/*
 *库文件
 */
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct {
  ngx_str_t filename;
}ngx_http_out_filecontent_loc_conf_t;

static char *ngx_http_out(ngx_conf_t * cf,ngx_command_t * cmd,void * conf);
/*命令集数组*/
static ngx_command_t ngx_http_out_command[] = {
	{ ngx_string("out"),
	  NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
	  ngx_http_out, /*注册函数*/
	  0,/*没有offset, 只支持一个context*/
	  0, /*没有offset, 只有配置结构体存在的时候，才会用到*/
	  NULL},
	 ngx_null_command 
};
 

/*上下文结构体*/
static ngx_http_module_t ngx_http_out_module_ctx = {
	NULL,		/*preconfiguration-------------*/
	NULL,		/*postconfiguration------------*/
	NULL,		/*create main configuration----*/
	NULL,		/*init-------------------------*/
	NULL,		/*create server configuration--*/
	NULL,		/*merge------------------------*/
	NULL,		/*create location configuration*/
	NULL		/*merge------------------------*/
};

/*模块结构体，注:模块结构体不能设置为static*/
ngx_module_t ngx_http_out_module = {
	NGX_MODULE_V1,
	&ngx_http_out_module_ctx,
	ngx_http_out_command,
	NGX_HTTP_MODULE,
	NULL,	/*init master*/
	NULL,   /*init module*/
	NULL,	/*init process*/
	NULL,	/*init thread*/
	NULL,	/*out thread*/
	NULL,	/*out process*/
	NULL,	/*out master*/
	NGX_MODULE_V1_PADDING
};

ngx_str_t filename;

/*handler function*/
static ngx_int_t ngx_http_out_handler(ngx_http_request_t *r){
	ngx_buf_t *b;
	ngx_chain_t out;
	ngx_file_t file; /*文件描述符*/
	ssize_t n;
	u_char *buf;
	struct stat f_stat;

	file.name = filename; 
	if(stat((const char *)file.name.data, &f_stat) == -1){
		return -1;
	}
	
	buf = (u_char *) ngx_pcalloc(r->pool, f_stat.st_size);
	
	file.fd = ngx_open_file(file.name.data,NGX_FILE_RDONLY,NGX_FILE_OPEN,NGX_FILE_DEFAULT_ACCESS);
	
	n = ngx_read_file(&file, buf, f_stat.st_size, 0);
	if(n == NGX_ERROR) {
		return NGX_ERROR;
	}

	/*设置header 的content_type*/
	r->headers_out.content_type.len = sizeof("text/plain") - 1;
	r->headers_out.content_type.data = (u_char *)"text/plain"; /*注:这里的字符串需要强制转换为(u_char*)类型*/

	/*给b分配一块新的内存*/
	b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));

	/*将b 插入链表*/
	out.buf = b;
	out.next = NULL;
	
	b->pos = buf;	
	b->last = buf + f_stat.st_size;	/*pos>>last是用到的内存*/
	b->memory = 1;	
	b->last_buf = 1;	

	/*发送200作为响应返回*/
	r->headers_out.status = NGX_HTTP_OK;
	/*获得body的content_length*/
	r->headers_out.content_length_n = f_stat.st_size;
	/*发送headers*/
	ngx_http_send_header(r);

	/*发送body,同时返回output filter chain 的状态码*/
	return ngx_http_output_filter(r, &out);
	
}

/*handler注册函数*/
static char *ngx_http_out(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
	ngx_http_core_loc_conf_t *clcf;			/*定义一个指向core location的指针*/
	ngx_str_t *value;

	value = cf->args->elts;

	filename = value[1];
	
	/*注册handler*/
	clcf = ngx_http_conf_get_module_loc_conf(cf,ngx_http_core_module);
	clcf->handler = ngx_http_out_handler;

	return NGX_OK;
}







