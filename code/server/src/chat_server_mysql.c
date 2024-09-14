#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chat_server_mysql.h"
#include "logger.h"

Connection_pool *mysql_pool;

Connection_pool *create_mysql_pool()
{
    const char *server = "localhost";
    const char *user = "lwy";
    const char *password = "123456";
    const char *database = "chat_system";

    Connection_pool *pool = (Connection_pool *)malloc(sizeof(Connection_pool));
    if (!pool)
    {
        LOG_ERROR("Failed to alloc memory for connection pool");
        return NULL;
    }

    pthread_mutex_init(&pool->lock, NULL);
    pool->count = 0;

    for (int i = 0; i < MAX_CONNECTIONS; i++)
    {
        pool->connections[i] = mysql_init(NULL);
        if (pool->connections[i] == NULL)
        {
            fprintf(stderr, "mysql_init() failed\n");
            LOG_ERROR("mysql_init() failed");
            return NULL;
        }

        if (mysql_real_connect(pool->connections[i], server, user, password, database, 0, NULL, 0) == NULL)
        {
            fprintf(stderr, "mysql_real_connect() failed\n");
            LOG_ERROR("mysql_real_connect() failed");
            mysql_close(pool->connections[i]);
            return NULL;
        }

        MY_STMT *my_stmt = (MY_STMT *)malloc(sizeof(MY_STMT));
        // 初始化预处理句柄，防止sql注入
        my_stmt->stmt = mysql_stmt_init(pool->connections[i]);
        if (my_stmt->stmt == NULL)
        {
            fprintf(stderr, "mysql_stmt_init() failed\n");
            LOG_ERROR("mysql_stmt_init() failed");
            mysql_close(pool->connections[i]);
            return NULL;
        }

        my_stmt->using = 0;

        pool->stmts[i] = my_stmt;
        pool->count++;
    }

    return pool;
}

MYSQL_STMT *get_stmt()
{
    pthread_mutex_lock(&mysql_pool->lock);

    if (mysql_pool->count > 0)
    {
        for (int i = 0; i < MAX_CONNECTIONS; i++)
        {
            if (mysql_pool->stmts[i]->using == 0)
            {
                mysql_pool->stmts[i]->using = 1;
                mysql_pool->count--;
                pthread_mutex_unlock(&mysql_pool->lock);
                return mysql_pool->stmts[i]->stmt;
            }
        }
    }

    pthread_mutex_unlock(&mysql_pool->lock);
    return NULL;
}

void release_stmt(MYSQL_STMT *stmt)
{
    pthread_mutex_lock(&mysql_pool->lock);

    for (int i = 0; i < MAX_CONNECTIONS; i++)
    {
        if (mysql_pool->stmts[i]->stmt == stmt && mysql_pool->stmts[i]->using == 1)
        {
            mysql_pool->stmts[i]->using = 0;
            mysql_pool->count++;
            pthread_mutex_unlock(&mysql_pool->lock);
        }
    }
}

void destory_mysql_pool()
{
    pthread_mutex_lock(&mysql_pool->lock);

    for (int i = 0; i < MAX_CONNECTIONS; i++)
    {
        mysql_close(mysql_pool->connections[i]);
        free(mysql_pool->stmts[i]);
    }
    free(mysql_pool);

    pthread_mutex_unlock(&mysql_pool->lock);
}

int init_mysql()
{
    mysql_pool = create_mysql_pool();
    if (mysql_pool == NULL)
    {
        LOG_ERROR("init mysql pool failed");
        exit(1);
    }

    return EXIT_SUCCESS;
}

int insert_user(mysql_users *user)
{
    MYSQL_BIND bind[3];
    MYSQL_STMT * stmt;

    stmt = get_stmt();

    const char *insert_query = "INSERT INTO users (user_name, user_id, password) VALUES (?, ?, ?);";
    if (mysql_stmt_prepare(stmt, insert_query, strlen(insert_query)))
    {
        fprintf(stderr, "mysql_stmt_prepare() failed for insert\n");
        fprintf(stderr, "Error: %s\n", mysql_stmt_error(stmt));

        release_stmt(stmt);
        return EXIT_FAILURE;
    }

    // 绑定参数
    memset(bind, 0, sizeof(bind));

    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = user->user_name;
    bind[0].buffer_length = strlen(user->user_name);

    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = user->user_id;
    bind[1].buffer_length = strlen(user->user_id);

    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = user->password;
    bind[2].buffer_length = strlen(user->password);

    if (mysql_stmt_bind_param(stmt, bind))
    {
        fprintf(stderr, "mysql_stmt_bind_param() failed\n");
        fprintf(stderr, "Error: %s\n", mysql_stmt_error(stmt));
        release_stmt(stmt);
        return EXIT_FAILURE;
    }

    // 插入第一条记录
    if (mysql_stmt_execute(stmt))
    {
        fprintf(stderr, "mysql_stmt_execute() failed for insert\n");
        fprintf(stderr, "Error: %s\n", mysql_stmt_error(stmt));
    }
    else
    {
        printf("Inserted success\n");
    }

    if (mysql_stmt_reset(stmt))
    {
        fprintf(stderr, "mysql_stmt_reset() failed for insert\n");
        fprintf(stderr, "Error: %s\n", mysql_stmt_error(stmt));
    }

    release_stmt(stmt);
    return EXIT_SUCCESS;
}

int find_user_by_id(char *id, mysql_users *user)
{
    MYSQL_BIND bind[1];
    my_bool is_null[3];
    my_bool error[3];
    unsigned long length[3];
    MYSQL_BIND result[3];
    MYSQL_STMT * stmt;
    
    stmt = get_stmt();

    memset(bind, 0, sizeof(bind));

    const char *find_query = "select * from users where user_id = ?;";
    if (mysql_stmt_prepare(stmt, find_query, strlen(find_query)))
    {
        LOG_ERROR("mysql_stmt_prepare() failed for find_user_by_id");
        LOG_ERROR("Error: %s", mysql_stmt_error(stmt));
        release_stmt(stmt);
        return EXIT_FAILURE;
    }

    unsigned long user_id_len = strlen(id);

    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = id;
    bind[0].buffer_length = sizeof(id);
    bind[0].length = &user_id_len;
    bind[0].is_null = 0;

    if (mysql_stmt_bind_param(stmt, bind))
    {
        LOG_ERROR("mysql_stmt_bind_param() failed in find_user_by_id");
        LOG_ERROR("Error: %s", mysql_stmt_error(stmt));
        release_stmt(stmt);
        return EXIT_FAILURE;
    }

    if (mysql_stmt_execute(stmt))
    {
        LOG_ERROR("mysql_stmt_execute() failed for find_user_by_id");
        LOG_ERROR("Error: %s", mysql_stmt_error(stmt));
        release_stmt(stmt);
        return EXIT_FAILURE;
    }

    // 定义绑定结果集的结构体
    memset(result, 0, sizeof(result));

    result[0].buffer_type = MYSQL_TYPE_STRING;
    result[0].buffer = user->user_id;
    result[0].buffer_length = sizeof(user->user_id);
    result[0].is_null = &is_null[0];
    result[0].length = &length[0];
    result[0].error = &error[0];

    result[1].buffer_type = MYSQL_TYPE_STRING;
    result[1].buffer = user->user_name;
    result[1].buffer_length = sizeof(user->user_name);
    result[1].is_null = &is_null[1];
    result[1].length = &length[1];
    result[1].error = &error[1];

    result[2].buffer_type = MYSQL_TYPE_STRING;
    result[2].buffer = user->password;
    result[2].buffer_length = sizeof(user->password);
    result[2].is_null = &is_null[2];
    result[2].length = &length[2];
    result[2].error = &error[2];

    // 绑定结果集
    if (mysql_stmt_bind_result(stmt, result))
    {
        LOG_ERROR("mysql_stmt_bind_result() failed");
        LOG_ERROR("Error: %s", mysql_stmt_error(stmt));
        release_stmt(stmt);
        return EXIT_FAILURE;
    }

    // 获取结果
    while (mysql_stmt_fetch(stmt) == 0)
    {
        printf("====SQL============\n");
        if (!is_null[0])
        {
            LOG_DEBUG("id: %s", user->user_id);
        }
        if (!is_null[1])
        {
            LOG_DEBUG("name: %s", user->user_name);
        }
        if (!is_null[2])
        {
            LOG_DEBUG("passwd: %s", user->password);
        }
    }

    LOG_DEBUG("find_user_by_id success");

    if (mysql_stmt_reset(stmt))
    {
        LOG_ERROR("mysql_stmt_reset() failed for find");
        LOG_ERROR("Error: %s", mysql_stmt_error(stmt));
    }

    release_stmt(stmt);
    return EXIT_SUCCESS;
}

int find_user_by_name(char *name, mysql_users *user)
{
    return EXIT_SUCCESS;
}