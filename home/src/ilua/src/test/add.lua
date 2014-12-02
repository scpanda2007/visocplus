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

function printtable(a)
	print("[")
	print("table is -->",a,#a)
	for _,value in ipairs(a) do 
		if type(value)=="table" then
			printtable(value)
		else
			print("item::",value)
		end
	end
	print("]")
end

function funcrefxx()
	print("funcref....")
end

function get_funcref()
	return function() print("funcref iiiiiiiiiiiiiiii") end,1,funcrefxx,"xxxx"
end

function get_funcref1()
	return function() print("funcref xxxxxxxxxxxxx") end
end

print("test ---> ", test(1,2,3))
print("test4 ---> ", test4())
print("test2 --> ", test2(1,2,3))
print("test5 --> ", test5({1,2,3,{"s","ss",{1,1,1},{2,2,2}},"hehehe"}))

print("test6 --> ")
printtable(test6())
