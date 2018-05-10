# cinema

Não, isto não tem nada a ver com cinemas.

Unspecified: Should a request which could not be returned to the client
be logged in slog.txt / sbook.txt, or should it be freed?

If it should be freed:
    Go to worker() in workers.c and check answer_client(),
    if it fails call free_reserved_seats() instead of logging the request.