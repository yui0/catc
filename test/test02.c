int func(int x)
{
	return x*2;
}

int sum()
{
	int i, s;
	s = 0;
	i = 0;
	while (i < 10) {
		s = s + i;
		i = i + 1;
	}
	printf("sum=%d(%d)\n", s, i);
}

int main()
{
	int a;
	a = 10;
	printf("%d*2=%d\n", a, func(a));
	sum();
	if (a>0) return;
}
