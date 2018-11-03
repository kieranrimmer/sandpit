WITH shared_addr AS 
(
SELECT foo.account_a, foo.account_b, row_number() OVER (ORDER BY foo.account_a) AS shared
FROM (
SELECT 
  a.account_id as account_a
, b.account_id as account_b
FROM
service_a a
JOIN
service_b b
ON 
a.email_address = b.email_address
GROUP BY a.account_id, b.account_id
) foo
)
SELECT
row_number() OVER (PARTITION BY account_id order by email_address),
email_address
FROM
(
SELECT
service_a.* FROM service_a
LEFT OUTER JOIN 
shared_addr
ON
shared_addr.account_a = service_a.account_id
WHERE shared_addr.account_b IS NULL
	
UNION ALL

SELECT
service_b.* FROM service_b
LEFT OUTER JOIN 
shared_addr
ON
shared_addr.account_b = service_b.account_id
WHERE shared_addr.account_a IS NULL
	
UNION ALL
(
SELECT
shared_addr.shared,
service_b.email_address
FROM service_b
JOIN 
shared_addr
ON
shared_addr.account_b = service_b.account_id

UNION

SELECT
shared_addr.shared,
service_a.email_address
FROM service_a
JOIN 
shared_addr
ON
shared_addr.account_a = service_a.account_id
)
) bar
;