function add(a , b)
	print("a:",a,"b:",b)
	return a+b
end

function addstring(a , b)
	print("a:",a,"b:",b)
	return a..b
end

function gettable(a)
	return {1,2,3}
end

print("test ---> ", test(1,2,3))
print("test2 --> ", test2(1,2,3))
