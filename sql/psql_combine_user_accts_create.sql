
CREATE TABLE public.service_a
(
    account_id bigint NOT NULL,
    email_address character varying(255) COLLATE pg_catalog."default"
)
WITH (
    OIDS = FALSE
)
TABLESPACE pg_default;

CREATE TABLE public.service_b
(
    account_id bigint NOT NULL,
    email_address character varying(255) COLLATE pg_catalog."default"
)
WITH (
    OIDS = FALSE
)
TABLESPACE pg_default;


INSERT INTO public.service_a(
	account_id, email_address)
 	VALUES (1, 'a@foo.com');
INSERT INTO public.service_a(
	account_id, email_address) VALUES
			(1, 'b@foo.com');
			INSERT INTO public.service_a(
	account_id, email_address) VALUES
			(2, 'c@foo.com');
			;
			
INSERT INTO public.service_b(
	account_id, email_address)
	VALUES (3, 'a@foo.com');
INSERT INTO public.service_b(
	account_id, email_address)	VALUES 		(3, 'a@bar.com');
INSERT INTO public.service_b(
	account_id, email_address)	VALUES 		(4, 'd@foo.com');
			