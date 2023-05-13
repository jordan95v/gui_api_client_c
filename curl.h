struct StringBuffer
{
    char *data;
    size_t size;
};

// Structure pour stocker la réponse de la requête curl
void call(const char *url, struct StringBuffer *buffer);
