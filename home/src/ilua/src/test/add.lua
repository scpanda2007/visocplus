function add(a , b)
	print("a:",a,"b:",b)
	return a+b
end

function addstring(a , b)
	print("a:",a,"b:",b)
	return a..b
end

function gettable(a)
	return {1,2,3,{"s","ss",{1,1,1},{2,2,2}},"hehehe"}
end

function gettable0()
	return {1,2,3,{"s","ss",{1,1,1},{2,2,2}},"hehehe"}
end

print("test ---> ", test(1,2,3))
print("test4 ---> ", test4())
print("test2 --> ", test2(1,2,3))
print("test5 --> ", test5({1,2,3,{"s","ss",{1,1,1},{2,2,2}},"hehehe"}))