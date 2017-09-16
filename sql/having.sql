select name, count(name) from t group by name having count (*) > 1;
