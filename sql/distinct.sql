select * from (select distinct name, count(name) from t group by name) as a where count > 1;
