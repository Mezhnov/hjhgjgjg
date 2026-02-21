

@Composable
fun ProfilePhotoStep(
    userData: UserData,
    onUserDataChanged: (UserData) -> Unit,
    onComplete: () -> Unit
) {
    var bio by remember { mutableStateOf(userData.bio) }
    var username by remember { mutableStateOf(userData.username) }

    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(horizontal = 32.dp),
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        Spacer(modifier = Modifier.height(40.dp))

        Box(contentAlignment = Alignment.Center) {
            Box(
                modifier = Modifier
                    .size(120.dp)
                    .clip(CircleShape)
                    .background(
                        brush = Brush.linearGradient(
                            colors = listOf(MessengerColors.GradientStart, MessengerColors.GradientEnd)
                        )
                    )
                    .clickable { /* TODO: Открыть галерею */ },
                contentAlignment = Alignment.Center
            ) {
                if (userData.firstName.isNotEmpty()) {
                    Text(
                        text = userData.firstName.first().uppercase() +
                                (userData.lastName.firstOrNull()?.uppercase() ?: ""),
                        fontSize = 40.sp,
                        color = Color.White,
                        fontWeight = FontWeight.Bold
                    )
                } else {
                    Icon(
                        imageVector = Icons.Default.Person,
                        contentDescription = null,
                        tint = Color.White,
                        modifier = Modifier.size(60.dp)
                    )
                }
            }

            Box(
                modifier = Modifier
                    .align(Alignment.BottomEnd)
                    .offset(x = 4.dp, y = 4.dp)
                    .size(40.dp)
                    .clip(CircleShape)
                    .background(MessengerColors.Primary)
                    .border(3.dp, MessengerColors.Background, CircleShape)
                    .clickable { /* TODO: Открыть камеру */ },
                contentAlignment = Alignment.Center
            ) {
                Icon(
                    imageVector = Icons.Default.Add,
                    contentDescription = "Добавить фото",
                    tint = Color.White,
                    modifier = Modifier.size(20.dp)
                )
            }
        }

        Spacer(modifier = Modifier.height(24.dp))

        Text(
            text = "Ваш профиль",
            style = MaterialTheme.typography.headlineLarge,
            color = MessengerColors.TextPrimary,
            fontWeight = FontWeight.Bold
        )

        Spacer(modifier = Modifier.height(12.dp))

        Text(
            text = "Добавьте фото и расскажите о себе",
            style = MaterialTheme.typography.bodyMedium,
            color = MessengerColors.TextSecondary,
            textAlign = TextAlign.Center
        )

        Spacer(modifier = Modifier.height(32.dp))

        OutlinedTextField(
            value = username,
            onValueChange = {
                val filtered = it.filter { char ->
                    char.isLetterOrDigit() || char == '_'
                }.lowercase()
                if (filtered.length <= 32) {
                    username = filtered
                }
            },
            modifier = Modifier.fillMaxWidth(),
            label = { Text("Имя пользователя") },
            placeholder = { Text("username") },
            prefix = {
                Text(
                    "@",
                    color = MessengerColors.Primary,
                    fontWeight = FontWeight.SemiBold
                )
            },
            leadingIcon = {
                Icon(
                    imageVector = Icons.Default.AccountCircle,
                    contentDescription = null,
                    tint = MessengerColors.Primary
                )
            },
            shape = RoundedCornerShape(12.dp),
            colors = OutlinedTextFieldDefaults.colors(
                focusedBorderColor = MessengerColors.Primary,
                unfocusedBorderColor = MessengerColors.Divider,
                focusedContainerColor = MessengerColors.Surface,
                unfocusedContainerColor = MessengerColors.Surface
            ),
            supportingText = {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    if (username.length >= 5) {
                        Icon(
                            imageVector = Icons.Default.Check,
                            contentDescription = null,
                            tint = MessengerColors.Success,
                            modifier = Modifier.size(14.dp)
                        )
                        Spacer(modifier = Modifier.width(4.dp))
                        Text(
                            text = "Имя доступно",
                            style = MaterialTheme.typography.bodySmall,
                            color = MessengerColors.Success
                        )
                    } else {
                        Text(
                            text = "Минимум 5 символов. Можно a-z, 0-9, _",
                            style = MaterialTheme.typography.bodySmall,
                            color = MessengerColors.TextSecondary
                        )
                    }
                }
            },
            singleLine = true
        )

        Spacer(modifier = Modifier.height(16.dp))

        OutlinedTextField(
            value = bio,
            onValueChange = { if (it.length <= 70) bio = it },
            modifier = Modifier.fillMaxWidth(),
            label = { Text("О себе") },
            placeholder = { Text("Расскажите о себе...") },
            leadingIcon = {
                Icon(
                    imageVector = Icons.Outlined.Edit,
                    contentDescription = null,
                    tint = MessengerColors.Primary
                )
            },
            shape = RoundedCornerShape(12.dp),
            colors = OutlinedTextFieldDefaults.colors(
                focusedBorderColor = MessengerColors.Primary,
                unfocusedBorderColor = MessengerColors.Divider,
                focusedContainerColor = MessengerColors.Surface,
                unfocusedContainerColor = MessengerColors.Surface
            ),
            supportingText = {
                Text(
                    text = "${bio.length}/70",
                    style = MaterialTheme.typography.bodySmall,
                    color = if (bio.length > 60) MessengerColors.Error else MessengerColors.TextSecondary,
                    modifier = Modifier.fillMaxWidth(),
                    textAlign = TextAlign.End
                )
            },
            maxLines = 3
        )

        Spacer(modifier = Modifier.weight(1f))

        Button(
            onClick = {
                onUserDataChanged(userData.copy(bio = bio, username = username))
                onComplete()
            },
            modifier = Modifier
                .fillMaxWidth()
                .height(54.dp),
            shape = RoundedCornerShape(12.dp),
            colors = ButtonDefaults.buttonColors(containerColor = MessengerColors.Primary)
        ) {
            Icon(
                imageVector = Icons.Default.Check,
                contentDescription = null,
                tint = Color.White
            )
            Spacer(modifier = Modifier.width(8.dp))
            Text(
                text = "Завершить регистрацию",
                style = MaterialTheme.typography.titleMedium,
                color = Color.White
            )
        }

        Spacer(modifier = Modifier.height(16.dp))

        TextButton(onClick = onComplete) {
            Text(
                text = "Пропустить",
                style = MaterialTheme.typography.bodyMedium,
                color = MessengerColors.Link
            )
        }

        Spacer(modifier = Modifier.height(32.dp))
    }
}

@Composable
fun SuccessScreen(
    userData: UserData,
    onLogout: () -> Unit
) {
    var isVisible by remember { mutableStateOf(false) }

    LaunchedEffect(Unit) {
        delay(100)
        isVisible = true
    }

    val scale by animateFloatAsState(
        targetValue = if (isVisible) 1f else 0f,
        animationSpec = spring(
            dampingRatio = Spring.DampingRatioMediumBouncy,
            stiffness = Spring.StiffnessLow
        ),
        label = "scale"
    )

    val alpha by animateFloatAsState(
        targetValue = if (isVisible) 1f else 0f,
        animationSpec = tween(500),
        label = "alpha"
    )

    Box(
        modifier = Modifier
            .fillMaxSize()
            .background(MessengerColors.Background)
    ) {
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(horizontal = 32.dp)
                .alpha(alpha),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.Center
        ) {
            Box(
                modifier = Modifier
                    .size(120.dp)
                    .scale(scale)
                    .clip(CircleShape)
                    .background(
                        brush = Brush.linearGradient(
                            colors = listOf(MessengerColors.Success, Color(0xFF2ECC71))
                        )
                    ),
                contentAlignment = Alignment.Center
            ) {
                Icon(
                    imageVector = Icons.Default.Check,
                    contentDescription = "Успех",
                    tint = Color.White,
                    modifier = Modifier.size(60.dp)
                )
            }

            Spacer(modifier = Modifier.height(32.dp))

            Text(
                text = if (userData.isNewUser) "Добро пожаловать!" else "С возвращением!",
                style = MaterialTheme.typography.headlineLarge,
                color = MessengerColors.TextPrimary,
                fontWeight = FontWeight.Bold
            )

            Spacer(modifier = Modifier.height(24.dp))

            Card(
                modifier = Modifier.fillMaxWidth(),
                shape = RoundedCornerShape(20.dp),
                colors = CardDefaults.cardColors(containerColor = MessengerColors.Surface),
                elevation = CardDefaults.cardElevation(defaultElevation = 4.dp)
            ) {
                Column(
                    modifier = Modifier.padding(24.dp),
                    horizontalAlignment = Alignment.CenterHorizontally
                ) {
                    Box(
                        modifier = Modifier
                            .size(80.dp)
                            .clip(CircleShape)
                            .background(
                                brush = Brush.linearGradient(
                                    colors = listOf(MessengerColors.GradientStart, MessengerColors.GradientEnd)
                                )
                            ),
                        contentAlignment = Alignment.Center
                    ) {
                        if (userData.firstName.isNotEmpty()) {
                            Text(
                                text = userData.firstName.first().uppercase(),
                                fontSize = 32.sp,
                                color = Color.White,
                                fontWeight = FontWeight.Bold
                            )
                        } else {
                            Icon(
                                imageVector = Icons.Default.Person,
                                contentDescription = null,
                                tint = Color.White,
                                modifier = Modifier.size(40.dp)
                            )
                        }
                    }

                    Spacer(modifier = Modifier.height(16.dp))

                    Text(
                        text = "${userData.firstName} ${userData.lastName}".trim().ifEmpty { "Пользователь" },
                        style = MaterialTheme.typography.headlineMedium,
                        color = MessengerColors.TextPrimary,
                        fontWeight = FontWeight.Bold
                    )

                    if (userData.username.isNotEmpty()) {
                        Spacer(modifier = Modifier.height(4.dp))
                        Text(
                            text = "@${userData.username}",
                            style = MaterialTheme.typography.bodyLarge,
                            color = MessengerColors.Link
                        )
                    }

                    if (userData.phoneNumber.isNotEmpty()) {
                        Spacer(modifier = Modifier.height(8.dp))
                        Row(verticalAlignment = Alignment.CenterVertically) {
                            Icon(
                                imageVector = Icons.Default.Phone,
                                contentDescription = null,
                                tint = MessengerColors.TextSecondary,
                                modifier = Modifier.size(16.dp)
                            )
                            Spacer(modifier = Modifier.width(6.dp))
                            Text(
                                text = userData.phoneNumber,
                                style = MaterialTheme.typography.bodyMedium,
                                color = MessengerColors.TextSecondary
                            )
                        }
                    }

                    if (userData.birthDay.isNotEmpty() && userData.birthMonth.isNotEmpty()) {
                        Spacer(modifier = Modifier.height(8.dp))
                        val months = listOf(
                            "января", "февраля", "марта", "апреля", "мая", "июня",
                            "июля", "августа", "сентября", "октября", "ноября", "декабря"
                        )
                        val monthName = months.getOrElse(userData.birthMonth.toInt() - 1) { "" }
                        Row(verticalAlignment = Alignment.CenterVertically) {
                            Icon(
                                imageVector = Icons.Default.Cake,
                                contentDescription = null,
                                tint = MessengerColors.TextSecondary,
                                modifier = Modifier.size(16.dp)
                            )
                            Spacer(modifier = Modifier.width(6.dp))
                            Text(
                                text = "${userData.birthDay} $monthName ${userData.birthYear}",
                                style = MaterialTheme.typography.bodyMedium,
                                color = MessengerColors.TextSecondary
                            )
                        }
                    }

                    if (userData.bio.isNotEmpty()) {
                        Spacer(modifier = Modifier.height(16.dp))
                        HorizontalDivider(color = MessengerColors.Divider)
                        Spacer(modifier = Modifier.height(16.dp))
                        Text(
                            text = userData.bio,
                            style = MaterialTheme.typography.bodyMedium,
                            color = MessengerColors.TextPrimary,
                            textAlign = TextAlign.Center
                        )
                    }
                }
            }

            Spacer(modifier = Modifier.height(32.dp))

            Text(
                text = if (userData.isNewUser) 
                    "Ваш аккаунт успешно создан!\nТеперь вы можете начать общение." 
                else 
                    "Вы успешно вошли в аккаунт!\nТеперь вы можете продолжить общение.",
                style = MaterialTheme.typography.bodyMedium,
                color = MessengerColors.TextSecondary,
                textAlign = TextAlign.Center,
                lineHeight = 22.sp
            )

            Spacer(modifier = Modifier.height(40.dp))

            Button(
                onClick = { /* TODO: Перейти к чатам */ },
                modifier = Modifier
                    .fillMaxWidth()
                    .height(54.dp),
                shape = RoundedCornerShape(12.dp),
                colors = ButtonDefaults.buttonColors(containerColor = MessengerColors.Primary)
            ) {
                Icon(
                    imageVector = Icons.AutoMirrored.Filled.ArrowForward,
                    contentDescription = null,
                    tint = Color.White
                )
                Spacer(modifier = Modifier.width(8.dp))
                Text(
                    text = if (userData.isNewUser) "Начать общение" else "Продолжить",
                    style = MaterialTheme.typography.titleMedium,
                    color = Color.White
                )
            }

            Spacer(modifier = Modifier.height(16.dp))

            TextButton(onClick = onLogout) {
                Icon(
                    imageVector = Icons.Default.ExitToApp,
                    contentDescription = null,
                    tint = MessengerColors.Error,
                    modifier = Modifier.size(18.dp)
                )
                Spacer(modifier = Modifier.width(8.dp))
                Text(
                    text = "Выйти из аккаунта",
                    style = MaterialTheme.typography.bodyMedium,
                    color = MessengerColors.Error
                )
            }
        }
    }
}

@Composable
fun MessengerLogo() {
    val infiniteTransition = rememberInfiniteTransition(label = "logo_animation")

    val rotation by infiniteTransition.animateFloat(
        initialValue = -5f,
        targetValue = 5f,
        animationSpec = infiniteRepeatable(
            animation = tween(2000, easing = FastOutSlowInEasing),
            repeatMode = RepeatMode.Reverse
        ),
        label = "rotation"
    )

    Box(
        modifier = Modifier
            .size(100.dp)
            .rotate(rotation)
            .clip(CircleShape)
            .background(
                brush = Brush.linearGradient(
                    colors = listOf(MessengerColors.GradientStart, MessengerColors.GradientEnd)
                )
            ),
        contentAlignment = Alignment.Center
    ) {
        AsyncImage(
            model = OnboardingImages.MESSENGER,
            contentDescription = "Messenger Logo",
            modifier = Modifier
                .size(90.dp)
                .clip(CircleShape),
            contentScale = ContentScale.Crop
        )
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun CountryPickerDialog(
    countries: List<Country>,
    selectedCountry: Country,
    onCountrySelected: (Country) -> Unit,
    onDismiss: () -> Unit
) {
    var searchQuery by remember { mutableStateOf("") }

    val filteredCountries = remember(searchQuery) {
        if (searchQuery.isEmpty()) {
            countries
        } else {
            countries.filter { country ->
                country.name.contains(searchQuery, ignoreCase = true) ||
                        country.phoneCode.contains(searchQuery)
            }
        }
    }

    Dialog(
        onDismissRequest = onDismiss,
        properties = DialogProperties(usePlatformDefaultWidth = false)
    ) {
        Surface(
            modifier = Modifier
                .fillMaxSize()
                .padding(top = 48.dp),
            shape = RoundedCornerShape(topStart = 20.dp, topEnd = 20.dp),
            color = MessengerColors.Background
        ) {
            Column {
                Surface(
                    color = MessengerColors.Surface,
                    shadowElevation = 2.dp
                ) {
                    Column(modifier = Modifier.padding(20.dp)) {
                        Row(
                            modifier = Modifier.fillMaxWidth(),
                            horizontalArrangement = Arrangement.SpaceBetween,
                            verticalAlignment = Alignment.CenterVertically
                        ) {
                            Text(
                                text = "Выберите страну",
                                style = MaterialTheme.typography.headlineMedium,
                                color = MessengerColors.TextPrimary,
                                fontWeight = FontWeight.Bold
                            )
                            IconButton(onClick = onDismiss) {
                                Icon(
                                    imageVector = Icons.Default.Close,
                                    contentDescription = "Закрыть",
                                    tint = MessengerColors.TextSecondary
                                )
                            }
                        }

                        Spacer(modifier = Modifier.height(16.dp))

                        OutlinedTextField(
                            value = searchQuery,
                            onValueChange = { searchQuery = it },
                            modifier = Modifier.fillMaxWidth(),
                            placeholder = {
                                Text(text = "Поиск страны...", color = MessengerColors.TextSecondary)
                            },
                            leadingIcon = {
                                Icon(
                                    imageVector = Icons.Default.Search,
                                    contentDescription = "Поиск",
                                    tint = MessengerColors.TextSecondary
                                )
                            },
                            trailingIcon = {
                                AnimatedVisibility(
                                    visible = searchQuery.isNotEmpty(),
                                    enter = fadeIn() + scaleIn(),
                                    exit = fadeOut() + scaleOut()
                                ) {
                                    IconButton(onClick = { searchQuery = "" }) {
                                        Icon(
                                            imageVector = Icons.Default.Clear,
                                            contentDescription = "Очистить",
                                            tint = MessengerColors.TextSecondary
                                        )
                                    }
                                }
                            },
                            shape = RoundedCornerShape(12.dp),
                            colors = OutlinedTextFieldDefaults.colors(
                                focusedBorderColor = MessengerColors.Primary,
                                unfocusedBorderColor = MessengerColors.Divider,
                                focusedContainerColor = MessengerColors.Surface,
                                unfocusedContainerColor = MessengerColors.Surface
                            ),
                            singleLine = true
                        )
                    }
                }

                LazyColumn(modifier = Modifier.fillMaxSize()) {
                    items(filteredCountries) { country ->
                        CountryItem(
                            country = country,
                            isSelected = country == selectedCountry,
                            onClick = { onCountrySelected(country) }
                        )
                    }
                }
            }
        }
    }
}

@Composable
fun CountryItem(
    country: Country,
    isSelected: Boolean,
    onClick: () -> Unit
) {
    val backgroundColor by animateColorAsState(
        targetValue = if (isSelected) {
            MessengerColors.Primary.copy(alpha = 0.1f)
        } else {
            MessengerColors.Surface
        },
        label = "background_color"
    )

    Surface(
        modifier = Modifier
            .fillMaxWidth()
            .clickable(onClick = onClick),
        color = backgroundColor
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 20.dp, vertical = 16.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Text(text = country.flag, fontSize = 32.sp)

            Spacer(modifier = Modifier.width(16.dp))

            Column(modifier = Modifier.weight(1f)) {
                Text(
                    text = country.name,
                    style = MaterialTheme.typography.bodyLarge,
                    color = MessengerColors.TextPrimary,
                    fontWeight = if (isSelected) FontWeight.SemiBold else FontWeight.Normal
                )
            }

            Text(
                text = country.phoneCode,
                style = MaterialTheme.typography.bodyMedium,
                color = MessengerColors.Primary,
                fontWeight = FontWeight.Medium
            )

            AnimatedVisibility(
                visible = isSelected,
                enter = fadeIn() + scaleIn(),
                exit = fadeOut() + scaleOut()
            ) {
                Row {
                    Spacer(modifier = Modifier.width(12.dp))
                    Icon(
                        imageVector = Icons.Default.Check,
                        contentDescription = "Выбрано",
                        tint = MessengerColors.Primary,
                        modifier = Modifier.size(22.dp)
                    )
                }
            }
        }
    }

    HorizontalDivider(
        modifier = Modifier.padding(start = 68.dp),
        color = MessengerColors.Divider,
        thickness = 0.5.dp
    )
}
