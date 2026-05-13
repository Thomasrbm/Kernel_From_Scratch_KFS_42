

void ft_memcpy(void *dst, void *src, int size)
{
    char *d = (char *)dst;
    char *s = (char *)src;
    while (size--)
        *d++ = *s++;
}