extern volatile void exit (int);
extern double load_average (void);

double max_load_average = -1;
unsigned int job_slots_used = 0;


int
main ()
{
  printf ("load_average () = %.2f\n", load_average ());
  exit (0);
}

void
error (char *fmt, ...)
{
  exit (1);
}

void 
perror_with_name (char *a, char *b)
{
  exit (1);
}

void
wait_for_children (int i)
{
  return;
}
